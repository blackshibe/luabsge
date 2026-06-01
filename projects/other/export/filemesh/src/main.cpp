// Roblox FileMesh -> Wavefront OBJ converter.
//
// Reference: https://devforum.roblox.com/t/roblox-filemesh-format-specification/326114/1
//
// Supports:
//   v1.00, v1.01 : ASCII text format (positions, normals, UVs)
//   v2.00        : binary, no LODs
//   v3.00, v3.01 : binary, LODs (we keep LOD 0 only)
//   v4.00, v4.01 : binary, LODs + bones + subsets (geometry only)
//   v5.00        : best-effort; emits whatever geometry can be decoded

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <string>
#include <vector>

namespace {

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
    float tu, tv;
};

struct Face {
    uint32_t a, b, c;
};

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------

bool read_entire_file(const char* path, std::vector<uint8_t>& out) {
    FILE* file = std::fopen(path, "rb");
    if (!file) {
        std::fprintf(stderr, "error: could not open '%s'\n", path);
        return false;
    }
    std::fseek(file, 0, SEEK_END);
    long length = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    if (length < 0) {
        std::fclose(file);
        return false;
    }
    out.resize(static_cast<size_t>(length));
    size_t read = std::fread(out.data(), 1, out.size(), file);
    std::fclose(file);
    out.resize(read);
    return true;
}

// ---------------------------------------------------------------------------
// Header detection
// ---------------------------------------------------------------------------

// Reads `version X.YY` from the start of the buffer. Returns the byte offset
// of the byte immediately after the trailing newline, or 0 on failure.
size_t parse_version(const std::vector<uint8_t>& buf, int& major, int& minor) {
    // Header is "version M.NN" followed by either \n or \r\n.
    if (buf.size() < 13) return 0;
    if (std::memcmp(buf.data(), "version ", 8) != 0) return 0;

    size_t i = 8;
    auto digit = [&](uint8_t c) { return c >= '0' && c <= '9'; };

    int m = 0;
    while (i < buf.size() && digit(buf[i])) { m = m * 10 + (buf[i] - '0'); ++i; }
    if (i >= buf.size() || buf[i] != '.') return 0;
    ++i; // consume '.'

    int n = 0;
    int n_digits = 0;
    while (i < buf.size() && digit(buf[i])) { n = n * 10 + (buf[i] - '0'); ++i; ++n_digits; }
    if (n_digits == 0) return 0;
    // Normalize "1.0" -> 0, "1.01" -> 1, "1.10" -> 10, etc.
    if (n_digits == 1) n *= 10;

    // Consume trailing newline (\n or \r\n).
    if (i < buf.size() && buf[i] == '\r') ++i;
    if (i < buf.size() && buf[i] == '\n') ++i;

    major = m;
    minor = n;
    return i;
}

// ---------------------------------------------------------------------------
// V1 text parser
// ---------------------------------------------------------------------------

// V1 layout after the version line:
//   <numFaces>\n
//   [px,py,pz][nx,ny,nz][tu,tv,tw][px,py,pz]...  (3 verts per face, contiguous)
// V1.00 stores positions scaled by 2x — divide by 2 on read.
bool parse_v1(const std::vector<uint8_t>& buf, size_t offset, bool scale_half,
              std::vector<Vertex>& verts, std::vector<Face>& faces) {
    // Treat the buffer as a C string for sscanf parsing.
    std::string text(reinterpret_cast<const char*>(buf.data() + offset),
                     buf.size() - offset);
    const char* p = text.c_str();
    const char* end = p + text.size();

    uint32_t num_faces = 0;
    int consumed = 0;
    if (std::sscanf(p, " %u %n", &num_faces, &consumed) != 1) {
        std::fprintf(stderr, "v1: failed to read face count\n");
        return false;
    }
    p += consumed;

    verts.reserve(num_faces * 3);
    faces.reserve(num_faces);

    for (uint32_t f = 0; f < num_faces; ++f) {
        uint32_t base = static_cast<uint32_t>(verts.size());
        for (int v = 0; v < 3; ++v) {
            Vertex vert{};
            // Position [x,y,z]
            if (std::sscanf(p, " [%f,%f,%f] %n", &vert.px, &vert.py, &vert.pz, &consumed) != 3) {
                std::fprintf(stderr, "v1: bad position at face %u vert %d\n", f, v);
                return false;
            }
            p += consumed;
            // Normal [x,y,z]
            if (std::sscanf(p, " [%f,%f,%f] %n", &vert.nx, &vert.ny, &vert.nz, &consumed) != 3) {
                std::fprintf(stderr, "v1: bad normal at face %u vert %d\n", f, v);
                return false;
            }
            p += consumed;
            // UV [u,v,w]; w is always 0 in practice.
            float tw = 0.0f;
            if (std::sscanf(p, " [%f,%f,%f] %n", &vert.tu, &vert.tv, &tw, &consumed) != 3) {
                std::fprintf(stderr, "v1: bad uv at face %u vert %d\n", f, v);
                return false;
            }
            p += consumed;

            if (scale_half) {
                vert.px *= 0.5f;
                vert.py *= 0.5f;
                vert.pz *= 0.5f;
            }
            verts.push_back(vert);
        }
        faces.push_back({base, base + 1, base + 2});
        if (p >= end) break;
    }
    return true;
}

// ---------------------------------------------------------------------------
// V2 / V3 / V4 binary parsers
// ---------------------------------------------------------------------------

// Pulls a struct of type T from `buf` starting at `cursor` and advances it.
template <typename T>
bool read_pod(const std::vector<uint8_t>& buf, size_t& cursor, T& out) {
    if (cursor + sizeof(T) > buf.size()) return false;
    std::memcpy(&out, buf.data() + cursor, sizeof(T));
    cursor += sizeof(T);
    return true;
}

// Reads a vertex of arbitrary stride. Position/normal/UV are at fixed offsets;
// any trailing tangent/color bytes are skipped via stride.
bool read_vertex(const std::vector<uint8_t>& buf, size_t base, size_t stride,
                 Vertex& v) {
    if (base + sizeof(float) * 8 > buf.size()) return false;
    std::memcpy(&v.px, buf.data() + base + 0,  sizeof(float));
    std::memcpy(&v.py, buf.data() + base + 4,  sizeof(float));
    std::memcpy(&v.pz, buf.data() + base + 8,  sizeof(float));
    std::memcpy(&v.nx, buf.data() + base + 12, sizeof(float));
    std::memcpy(&v.ny, buf.data() + base + 16, sizeof(float));
    std::memcpy(&v.nz, buf.data() + base + 20, sizeof(float));
    std::memcpy(&v.tu, buf.data() + base + 24, sizeof(float));
    std::memcpy(&v.tv, buf.data() + base + 28, sizeof(float));
    (void)stride;
    return true;
}

#pragma pack(push, 1)
struct HeaderV2 {
    uint16_t sizeof_header;   // 12
    uint8_t  sizeof_vertex;   // 36 (no color) or 40 (with color)
    uint8_t  sizeof_face;     // 12
    uint32_t num_verts;
    uint32_t num_faces;
};

struct HeaderV3 {
    uint16_t sizeof_header;   // 16
    uint8_t  sizeof_vertex;
    uint8_t  sizeof_face;
    uint16_t sizeof_lod;      // 4
    uint16_t num_lods;
    uint32_t num_verts;
    uint32_t num_faces;
};

struct HeaderV4 {
    uint16_t sizeof_header;   // 24
    uint16_t lod_type;
    uint32_t num_verts;
    uint32_t num_faces;
    uint16_t num_lods;
    uint16_t num_bones;
    uint32_t sizeof_bone_names;
    uint16_t num_subsets;
    uint8_t  num_high_quality_lods;
    uint8_t  unused;
};
#pragma pack(pop)

bool parse_v2(const std::vector<uint8_t>& buf, size_t offset,
              std::vector<Vertex>& verts, std::vector<Face>& faces) {
    HeaderV2 h{};
    size_t cursor = offset;
    if (!read_pod(buf, cursor, h)) return false;

    // The header field may report its own logical size; trust it and skip
    // ahead to honour any unknown trailing fields the writer added.
    cursor = offset + h.sizeof_header;

    verts.resize(h.num_verts);
    for (uint32_t i = 0; i < h.num_verts; ++i) {
        if (!read_vertex(buf, cursor, h.sizeof_vertex, verts[i])) return false;
        cursor += h.sizeof_vertex;
    }
    faces.resize(h.num_faces);
    for (uint32_t i = 0; i < h.num_faces; ++i) {
        if (cursor + h.sizeof_face > buf.size()) return false;
        std::memcpy(&faces[i].a, buf.data() + cursor + 0, 4);
        std::memcpy(&faces[i].b, buf.data() + cursor + 4, 4);
        std::memcpy(&faces[i].c, buf.data() + cursor + 8, 4);
        cursor += h.sizeof_face;
    }
    return true;
}

bool parse_v3(const std::vector<uint8_t>& buf, size_t offset,
              std::vector<Vertex>& verts, std::vector<Face>& faces) {
    HeaderV3 h{};
    size_t cursor = offset;
    if (!read_pod(buf, cursor, h)) return false;
    cursor = offset + h.sizeof_header;

    verts.resize(h.num_verts);
    for (uint32_t i = 0; i < h.num_verts; ++i) {
        if (!read_vertex(buf, cursor, h.sizeof_vertex, verts[i])) return false;
        cursor += h.sizeof_vertex;
    }
    faces.resize(h.num_faces);
    for (uint32_t i = 0; i < h.num_faces; ++i) {
        if (cursor + h.sizeof_face > buf.size()) return false;
        std::memcpy(&faces[i].a, buf.data() + cursor + 0, 4);
        std::memcpy(&faces[i].b, buf.data() + cursor + 4, 4);
        std::memcpy(&faces[i].c, buf.data() + cursor + 8, 4);
        cursor += h.sizeof_face;
    }
    // LOD offsets follow; we keep LOD 0 which is already the full mesh.
    return true;
}

bool parse_v4(const std::vector<uint8_t>& buf, size_t offset,
              std::vector<Vertex>& verts, std::vector<Face>& faces) {
    HeaderV4 h{};
    size_t cursor = offset;
    if (!read_pod(buf, cursor, h)) return false;
    cursor = offset + h.sizeof_header;

    // v4 vertex stride is fixed at 40 bytes: 12 (pos) + 12 (nrm) + 8 (uv) +
    // 4 (tangent sbyte4) + 4 (rgba). Skinning (if num_bones > 0) adds 8 bytes
    // per vertex appended *after* the vertex array.
    const size_t kStride = 40;
    verts.resize(h.num_verts);
    for (uint32_t i = 0; i < h.num_verts; ++i) {
        if (!read_vertex(buf, cursor, kStride, verts[i])) return false;
        cursor += kStride;
    }
    if (h.num_bones > 0) {
        // Skip FileMeshSkinning[num_verts] — 8 bytes each (4 bone indices +
        // 4 weights).
        cursor += static_cast<size_t>(h.num_verts) * 8;
    }
    faces.resize(h.num_faces);
    for (uint32_t i = 0; i < h.num_faces; ++i) {
        if (cursor + 12 > buf.size()) return false;
        std::memcpy(&faces[i].a, buf.data() + cursor + 0, 4);
        std::memcpy(&faces[i].b, buf.data() + cursor + 4, 4);
        std::memcpy(&faces[i].c, buf.data() + cursor + 8, 4);
        cursor += 12;
    }
    // Trailing LOD offsets, bones, bone-name buffer, and subsets are ignored.
    return true;
}

// ---------------------------------------------------------------------------
// OBJ writer
// ---------------------------------------------------------------------------

bool write_obj(const char* path, const std::vector<Vertex>& verts,
               const std::vector<Face>& faces) {
    FILE* out = std::fopen(path, "wb");
    if (!out) {
        std::fprintf(stderr, "error: could not open '%s' for writing\n", path);
        return false;
    }
    std::fprintf(out, "# Converted from Roblox FileMesh\n");
    std::fprintf(out, "o mesh\n");
    for (const auto& v : verts) std::fprintf(out, "v %g %g %g\n",  v.px, v.py, v.pz);
    for (const auto& v : verts) std::fprintf(out, "vn %g %g %g\n", v.nx, v.ny, v.nz);
    for (const auto& v : verts) std::fprintf(out, "vt %g %g\n",    v.tu, 1.0f - v.tv);
    for (const auto& f : faces) {
        // OBJ indices are 1-based.
        uint32_t a = f.a + 1, b = f.b + 1, c = f.c + 1;
        std::fprintf(out, "f %u/%u/%u %u/%u/%u %u/%u/%u\n",
                     a, a, a, b, b, b, c, c, c);
    }
    std::fclose(out);
    return true;
}

// Replaces extension on `in_path` with `.obj`.
std::string derive_obj_path(const char* in_path) {
    std::string s(in_path);
    size_t dot = s.find_last_of('.');
    size_t slash = s.find_last_of("/\\");
    if (dot != std::string::npos && (slash == std::string::npos || dot > slash)) {
        s.erase(dot);
    }
    s += ".obj";
    return s;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <input.mesh> [output.obj]\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char* in_path = argv[1];
    std::string out_path = (argc >= 3) ? argv[2] : derive_obj_path(in_path);

    std::vector<uint8_t> buf;
    if (!read_entire_file(in_path, buf)) return EXIT_FAILURE;
    if (buf.empty()) {
        std::fprintf(stderr, "error: '%s' is empty\n", in_path);
        return EXIT_FAILURE;
    }

    int major = 0, minor = 0;
    size_t offset = parse_version(buf, major, minor);
    if (offset == 0) {
        std::fprintf(stderr, "error: not a Roblox mesh (missing 'version' header)\n");
        return EXIT_FAILURE;
    }
    std::printf("input : %s\nversion: %d.%02d\n", in_path, major, minor);

    std::vector<Vertex> verts;
    std::vector<Face>   faces;
    bool ok = false;
    switch (major) {
        case 1:
            // 1.00 stores positions scaled by 2; 1.01+ does not.
            ok = parse_v1(buf, offset, /*scale_half=*/(minor == 0), verts, faces);
            break;
        case 2:
            ok = parse_v2(buf, offset, verts, faces);
            break;
        case 3:
            ok = parse_v3(buf, offset, verts, faces);
            break;
        case 4:
        case 5: // best-effort: v5 header is a superset and often round-trips
            ok = parse_v4(buf, offset, verts, faces);
            break;
        default:
            std::fprintf(stderr, "error: unsupported mesh version %d.%02d\n", major, minor);
            return EXIT_FAILURE;
    }
    if (!ok) {
        std::fprintf(stderr, "error: failed to parse mesh body\n");
        return EXIT_FAILURE;
    }

    std::printf("verts : %zu\nfaces : %zu\n", verts.size(), faces.size());

    if (!write_obj(out_path.c_str(), verts, faces)) return EXIT_FAILURE;
    std::printf("output: %s\n", out_path.c_str());
    return EXIT_SUCCESS;
}
