import subprocess
import re

release_files = ['dmmap.h']

# Step 1: Extract the version and description from CHANGE_LOGS.md
def extract_version_and_description():
    version = None
    description = []
    with open('CHANGE_LOGS.md', 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith('## '):
                version_match = re.match(r'^##\s+(.+)$', line)
                if version_match:
                    version = version_match.group(1)  # Correctly extract the version title
            if line == "=======":
                break
            description.append(line)
    description_text = "\n".join(description)
    return version, description_text

# Step 2: Create and push the Git tag
def create_and_push_tag(version, description):
    try:
        subprocess.run(['git', 'tag', '-a', version, '-m', description], check=True)
        subprocess.run(['git', 'push', 'origin', version], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error during git tag/push: {e}")
        return False
    return True

# Step 3: Create a GitHub release and attach files if version contains "stable"
def create_github_release_and_attach_files(version):
    if "stable" in version:
        try:
            release_command = ['gh', 'release', 'create', version, *release_files, '--title', version, '--notes', '']
            subprocess.run(release_command, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error during GitHub release creation: {e}")
            return False
    return True

def main():
    version, description = extract_version_and_description()
    
    if not version:
        print("Error: No version found in CHANGE_LOGS.md or invalid format.")
        return
    
    print(f"Extracted version: {version}")
    print(f"Tag description:\n{description}")
    
    if create_and_push_tag(version, description):
        if create_github_release_and_attach_files(version):
            print(f"Successfully created and pushed version {version}, with attached files for the release.")
        else:
            print(f"Failed to create GitHub release or attach files for version {version}.")
    else:
        print(f"Failed to tag or push version {version}.")

if __name__ == "__main__":
    main()
