import subprocess
import re

release_files = ['dmmap.h']

# Step 1: Read CHANGE_LOGS.md and extract the version and description
def extract_version_and_description():
    version = None
    description = []
    with open('CHANGE_LOGS.md', 'r') as file:
        for line in file:
            line = line.strip()
            if line == "=======":
                break
            if line.startswith('## '):
                version_match = re.match(r'^##\s+(.+)$', line)
                if version_match:
                    version = version_match.group(1)
            description.append(line)
    description_text = "\n".join(description)
    return version, description_text

# Step 2: Add specific files and commit (if necessary)
def add_files_and_commit(version):
    try:
        # Stage files
        subprocess.run(['git', 'add', *release_files], check=True)
        
        # Check if there are any changes to commit
        result = subprocess.run(['git', 'status', '--porcelain'], capture_output=True, text=True)
        if not result.stdout.strip():
            print("No changes to commit.")
            return True
        
        # Commit the changes
        subprocess.run(['git', 'commit', '-m', f'Add artifacts for version {version}'], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error during git commit: {e}")
        return False
    return True

# Step 3: Create the Git tag and push
def create_and_push_tag(version, description):
    try:
        subprocess.run(['git', 'tag', '-a', version, '-m', description], check=True)
        subprocess.run(['git', 'push', 'origin', 'HEAD'], check=True)
        subprocess.run(['git', 'push', 'origin', version], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error during git tag/push: {e}")
        return False
    return True

def main():
    version, description = extract_version_and_description()
    
    if not version:
        print("Error: No version found in CHANGE_LOGS.md or invalid format.")
        return
    
    print(f"Extracted version: {version}")
    print(f"Tag description:\n{description}")
    
    if add_files_and_commit(version):
        if create_and_push_tag(version, description):
            print(f"Successfully tagged and pushed version {version}.")
        else:
            print(f"Failed to push tag for version {version}.")
    else:
        print(f"Failed to commit files for version {version}.")

if __name__ == "__main__":
    main()
