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

# Step 3: Create or update a GitHub release and attach files if version contains "stable"
def create_or_update_github_release(version, description):
    try:
        # Check if a release with this tag already exists
        result = subprocess.run(['gh', 'release', 'view', version], capture_output=True, text=True)
        release_exists = result.returncode == 0

        if release_exists:
            # Update the existing release
            subprocess.run(['gh', 'release', 'edit', version, '--title', version, '--notes', description], check=True)
        else:
            # Create a new release
            release_command = ['gh', 'release', 'create', version, *release_files, '--title', version, '--notes', description]
            subprocess.run(release_command, check=True)

        # Check if the "latest" release exists
        result_latest = subprocess.run(['gh', 'release', 'view', 'latest'], capture_output=True, text=True)
        latest_exists = result_latest.returncode == 0

        if "stable" in version:
            if latest_exists:
                # Update the "latest" release to point to this tag
                subprocess.run(['gh', 'release', 'edit', 'latest', '--tag', version, '--latest'], check=True)
            else:
                # Create a new "latest" release
                subprocess.run(['gh', 'release', 'create', version, *release_files, '--title', 'Latest Release', '--notes', description, '--latest'], check=True)

    except subprocess.CalledProcessError as e:
        print(f"Error during GitHub release creation or update: {e}")
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
        if create_or_update_github_release(version, description):
            print(f"Successfully created or updated the release for version {version}.")
        else:
            print(f"Failed to create or update GitHub release for version {version}.")
    else:
        print(f"Failed to tag or push version {version}.")

if __name__ == "__main__":
    main()
