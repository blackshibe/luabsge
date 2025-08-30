import csv
import os
import re
import urllib.request
import urllib.error
import ssl

# === CONFIGURATION - EDIT THESE VALUES ===
COOKIE = ""
MESH_FILE = "in/meshes.csv"                    
IMAGE_FILE = "in/images.csv"                    
OUTPUT_DIR = "out"                # Output directory
# ========================================

with open('.cookie', 'r') as file:
    COOKIE = file.read()

def extract_asset_id(rbxasset_url):
    """Extract numeric asset ID from URL"""
    if not rbxasset_url:
        return None
    match = re.search(r'rbxassetid://(\d+)', rbxasset_url)
    if match:
        return match.group(1)
    match = re.search(r'(\d+)', rbxasset_url)
    return match.group(1) if match else None

def download_asset(asset_id, filename, cookie, asset_type="generic"):
    """Download a single asset with type-specific handling"""
    if not asset_id:
        return False
    
    url = f"https://assetdelivery.roblox.com/v1/asset/?id={asset_id}"
    
    ssl_context = ssl.create_default_context()
    ssl_context.check_hostname = False
    ssl_context.verify_mode = ssl.CERT_NONE
    
    request = urllib.request.Request(url)
    request.add_header('User-Agent', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36')
    request.add_header('Cookie', cookie)
    request.add_header('Referer', 'https://www.roblox.com/')
    
    try:
        with urllib.request.urlopen(request, timeout=30, context=ssl_context) as response:
            if response.getcode() == 200:
                os.makedirs(os.path.dirname(filename), exist_ok=True)
                content = response.read()
                
                # Add appropriate file extension based on asset type
                if asset_type == "mesh":
                    if not filename.lower().endswith(('.mesh', '.obj', '.fbx')):
                        filename += '.mesh'  # Default mesh extension
                elif asset_type == "image":
                    # Try to detect image type from content
                    if content.startswith(b'\x89PNG'):
                        filename += '.png'
                    elif content.startswith(b'\xFF\xD8\xFF'):
                        filename += '.jpg'
                    elif content.startswith(b'GIF87a') or content.startswith(b'GIF89a'):
                        filename += '.gif'
                    elif content.startswith(b'RIFF') and content[8:12] == b'WEBP':
                        filename += '.webp'
                    elif not any(filename.lower().endswith(ext) for ext in ['.png', '.jpg', '.jpeg', '.gif', '.webp', '.bmp']):
                        filename += '.png'  # Default image extension
                
                with open(filename, 'wb') as f:
                    f.write(content)
                
                file_size = len(content)
                print(f"Downloaded {os.path.basename(filename)} (ID: {asset_id}, Type: {asset_type}, Size: {file_size} bytes)")
                return True
                
    except urllib.error.HTTPError as e:
        if e.code == 403:
            print(f"Access denied for {filename} (ID: {asset_id}) - Check cookie permissions")
        elif e.code == 404:
            print(f"Asset not found: {filename} (ID: {asset_id})")
        else:
            print(f"HTTP {e.code} for {filename} (ID: {asset_id})")
        return False
        
    except Exception as e:
        print(f"Error downloading {filename}: {e}")
        return False

def process_csv_file(csv_file, cookie, output_dir, asset_type="generic"):
    """Process a CSV file with specific asset type handling"""
    print(f"\n{'='*60}")
    print(f"Processing {asset_type.upper()} file: {csv_file}")
    print(f"{'='*60}")
    
    if not os.path.exists(csv_file):
        print(f"Error: File '{csv_file}' not found")
        return 0, 0, 0
    
    successful_downloads = 0
    failed_downloads = 0
    skipped_downloads = 0
    
    try:
        with open(csv_file, 'r', encoding='utf-8') as file:
            content = file.read()
            print(f"File content preview (first 200 chars):")
            print("-" * 40)
            print(content[:200] + "..." if len(content) > 200 else content)
            print("-" * 40)
            
            # Reset and parse
            file.seek(0)
            lines = file.readlines()
            
            if not lines:
                print("File is empty!")
                return 0, 0, 0
            
            print(f"Found {len(lines)} lines in file")
            
            for line_num, line in enumerate(lines, 1):
                line = line.strip()
                if not line:
                    print(f"Line {line_num}: Empty line, skipping")
                    skipped_downloads += 1
                    continue
                
                # Remove quotes if present
                line = line.replace('"', '').replace("'", "")
                
                # Try different splitting methods
                if ',' in line:
                    parts = line.split(',')
                elif ';' in line:
                    parts = line.split(';')
                elif '\t' in line:
                    parts = line.split('\t')
                else:
                    parts = line.split()
                
                if len(parts) >= 2:
                    asset_name = parts[0].strip()
                    asset_url = parts[1].strip()
                    
                    if not asset_name or asset_url.lower() == 'nil' or asset_url == '':
                        print(f"Line {line_num}: Skipping - empty name or URL")
                        skipped_downloads += 1
                        continue
                    
                    asset_id = extract_asset_id(asset_url)
                    if asset_id:
                        # Create type-specific subfolder
                        type_dir = os.path.join(output_dir, asset_type)
                        filename = os.path.join(type_dir, asset_name)
                        
                        if download_asset(asset_id, filename, cookie, asset_type):
                            successful_downloads += 1
                        else:
                            failed_downloads += 1
                    else:
                        print(f"Line {line_num}: Could not extract asset ID from '{asset_url}'")
                        failed_downloads += 1
                else:
                    print(f"Line {line_num}: Not enough columns (need at least 2) - '{line}'")
                    skipped_downloads += 1
                    
    except Exception as e:
        print(f"Error processing {csv_file}: {e}")
        return successful_downloads, failed_downloads, skipped_downloads
    
    return successful_downloads, failed_downloads, skipped_downloads

def main():
    # Create main output directory
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    total_successful = 0
    total_failed = 0
    total_skipped = 0
    
    # Process images CSV
    if os.path.exists(IMAGE_FILE):
        success, failed, skipped = process_csv_file(IMAGE_FILE, COOKIE, OUTPUT_DIR, "image")
        total_successful += success
        total_failed += failed
        total_skipped += skipped
    else:
        print(f"Warning: Image file '{IMAGE_FILE}' not found")
    
    # Process meshes CSV
    if os.path.exists(MESH_FILE):
        success, failed, skipped = process_csv_file(MESH_FILE, COOKIE, OUTPUT_DIR, "mesh")
        total_successful += success
        total_failed += failed
        total_skipped += skipped
    else:
        print(f"Warning: Mesh file '{MESH_FILE}' not found")
    
    # Print final summary
    print(f"\n{'='*60}")
    print("FINAL DOWNLOAD SUMMARY")
    print(f"{'='*60}")
    print(f"Successful downloads: {total_successful}")
    print(f"Failed downloads: {total_failed}")
    print(f"Skipped entries: {total_skipped}")
    print(f"Total processed: {total_successful + total_failed + total_skipped}")
    print(f"{'='*60}")
    
    if total_successful > 0:
        print(f"Files saved in: {os.path.abspath(OUTPUT_DIR)}")
        # Show created subdirectories
        for item in os.listdir(OUTPUT_DIR):
            item_path = os.path.join(OUTPUT_DIR, item)
            if os.path.isdir(item_path):
                file_count = len([f for f in os.listdir(item_path) if os.path.isfile(os.path.join(item_path, f))])
                print(f"  {item}/ - {file_count} files")

if __name__ == "__main__":
    main()