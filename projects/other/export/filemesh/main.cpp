#include <iostream>
#include <cstdint>

// struct FileMeshV2
// {
//     FileMeshHeaderV2 Header;
//     FileMeshVertex Verts[];
//     FileMeshFace Faces[];
// }

// struct FileMeshFace
// {
//     uint a; // 1st Vertex Index
//     uint b; // 2nd Vertex Index
//     uint c; // 3rd Vertex Index
// }

// struct FileMeshHeaderV2
// {
//     ushort sizeof_FileMeshHeaderV2;
//     byte   sizeof_FileMeshVertex;
//     byte   sizeof_FileMeshFace;

//     uint   numVerts;
//     uint   numFaces;
// };

// struct FileMeshVertex
// {
//    float px, py, pz;     // Position
//    float nx, ny, nz;     // Normal
//    float tu, tv;         // UV

//    short tx, ty, tz, ts; // Tangent Vector & Bi-Normal Direction
//    short r, g, b, a;     // RGBA Color Tinting (May not be present!)
// };

struct FileMeshHeaderV4 {
	uint16_t sizeof_FileMeshHeaderV4;
	uint16_t lodType;

	uint8_t numVerts;
	uint8_t numFaces;
	
	uint16_t numLodOffsets;
	uint16_t numBones;
	
	uint8_t  sizeof_boneNames;
	uint16_t numSubsets;

	uint8_t numHighQualityLODs;
	uint8_t unused;
};

// no easy way to deal with the dynamic sizes
struct FileMeshV4 {
	FileMeshHeaderV4 header;
	
	FileMeshVertex verts[numVerts];
	FileMeshSkinning? skinning[numVerts];
	
	FileMeshFace faces[numFaces];
	uint lodOffsets[numLodOffsets];

	FileMeshBone bones[numBones];
	byte boneNames[sizeof_boneNames];

	FileMeshSubset subsets[numSubsets];
};

struct FileMeshHeader {
    const char* Header[12];
};

const char* load_file(const char* filename) {
    char* buffer = NULL;
    long length;
    
    // Use binary mode to avoid line ending issues on Windows
    FILE* file = fopen(filename, "rb");  // Note: "rb" instead of "r"
    
    if (file) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        // Allocate space for content + null terminator
        buffer = (char*)malloc(length + 1);
        
        if (buffer) {
            size_t bytes_read = fread(buffer, 1, length, file);
            buffer[bytes_read] = '\0';  // Always add null terminator
        }
        
        fclose(file);
    }
    
    return buffer;
}

int main(int argc, char *argv[]) {

    // Read file at argv location
    const char* path = argv[1];
    printf("processing path: %s\n", path);

    // Parse file, parse version


    // Parse vertices data
    // Output OBJ equivalent
    return EXIT_SUCCESS;
}