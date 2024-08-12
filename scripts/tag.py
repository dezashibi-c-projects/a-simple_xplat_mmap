import subprocess
import re

release_files = ['dmmap.h']

# Step 1: Read CHANGE_LOGS.md and extract the version
def extract_version():
    version = None
    with open('CHANGE_LOGS.md', 'r') as file:
        for line in file:
            line = line.strip()
            if line == "=======":
                break
            if line.startswith('## '):
                version_match = re.match(r'^##\s+(.+)$', line)
                if version_match:
                    version = version_match.group(1)
                    break
    return version

# Step 2: Add specific files and commit
def add_files_and_commit(version):
    try:
        subprocess.run(['git', 'add', *release_files], check=True)
        subprocess.run(['git', 'commit', '-m', f'Add artifacts for version {version}'], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error during git commit: {e}")
        return False
    return True

# Step 3: Create the Git tag and push
def create_and_push_tag(version):
    try:
        subprocess.run(['git', 'tag', '-a', version, '-m', f'Tagging version {version}'], check=True)
        subprocess.run(['git', 'push', 'origin', 'HEAD'], check=True)
        subprocess.run(['git', 'push', 'origin', version], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error during git tag/push: {e}")
        return False
    return True

def main():
    version = extract_version()
    
    if not version:
        print("Error: No version found in CHANGE_LOGS.md or invalid format.")
        return
    
    print(f"Extracted version: {version}")
    
    if add_files_and_commit(version):
        if create_and_push_tag(version):
            print(f"Successfully tagged and pushed version {version}.")
        else:
            print(f"Failed to push tag for version {version}.")
    else:
        print(f"Failed to commit files for version {version}.")

if __name__ == "__main__":
    main()
