#!/bin/bash

printf "\n=== Check dependencies ===\n"

version="6.0.3"
inet_version="4.5.3"

# Function to check if a package is installed
is_package_installed() {
    dpkg -s "$1" &> /dev/null
    return $?
}

# Function to install a package
install_package() {
    echo "Installing $1..."
    sudo apt-get install -y "$1"
}

# List of required packages
required_packages=("build-essential" "clang" "lld" "gdb" "bison" "flex" "perl" "python3" "python3-pip" "python3-venv"
"qtbase5-dev" "qtchooser" "qt5-qmake" "qtbase5-dev-tools" "libqt5opengl5-dev" "libxml2-dev" "zlib1g-dev" "doxygen"
"graphviz" "libwebkit2gtk-4.0-37" "curl" "tar") # Replace with actual package names

# Check for required packages
missing_packages=()
for pkg in "${required_packages[@]}"; do
    if ! is_package_installed "$pkg"; then
        missing_packages+=("$pkg")
    fi
done

# Install missing packages if any
if [ ${#missing_packages[@]} -ne 0 ]; then
    echo "The following packages are missing and need to be installed: ${missing_packages[*]}"
    read -p "Do you want to install them now? (y/n/skip) " response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        for pkg in "${missing_packages[@]}"; do
            install_package "$pkg"
        done
    elif [[ "$response" =~ ^[Ss][Kk][Ii][Pp]$ ]]; then
        echo "Skipping installation of missing packages. Be aware that the installation may fail without these packages."
    else
        echo "Installation cannot proceed without these packages. Exiting."
        exit 1
    fi
fi

printf "\n=== Check Python modules ===\n"

# List of required Python modules
required_python_modules=("numpy" "pandas" "matplotlib" "scipy" "seaborn" "posix_ipc")

# Function to check if a Python module is installed
is_python_module_installed() {
    python3 -c "import $1" 2> /dev/null
    return $?
}

# Function to install a Python module and check if installation was successful
install_and_check_python_module() {
    echo "Installing $1..."
    python3 -m pip install --upgrade "$1"
    if is_python_module_installed "$1"; then
        echo "$1 installed successfully."
    else
        echo "Failed to install $1. Please check for errors and try manually."
        failed_installations+=("$1")
    fi
}

# Check for required Python modules
missing_python_modules=()
for module in "${required_python_modules[@]}"; do
    if ! is_python_module_installed "$module"; then
        missing_python_modules+=("$module")
    fi
done

create_venv() {
  echo "Creating venv..."
  default_venv_path="$HOME/venv/deterministic6g"
  read -p "Enter the venv path [$default_venv_path]: " venv_path
  venv_path=${venv_path:-$default_venv_path}
  python3 -m venv "$venv_path"
  source "$venv_path/bin/activate"
}

# Ask to install missing Python modules
failed_installations=()
if [ ${#missing_python_modules[@]} -ne 0 ]; then
    echo "The following Python modules are missing: ${missing_python_modules[*]}"
    read -p "Do you want to create a venv and install them now? (y/n/skip) " response
    if [[ "$response" =~ ^[Yy]$ ]]; then
      create_venv
        for module in "${missing_python_modules[@]}"; do
            install_and_check_python_module "$module"
        done
        if [ ${#failed_installations[@]} -eq 0 ]; then
            echo "All missing Python modules have been successfully installed."
        else
            echo "Failed to install the following modules: ${failed_installations[*]}"
            echo "Please install these modules manually."
            exit 1
        fi
    elif [[ "$response" =~ ^[Ss][Kk][Ii][Pp]$ ]]; then
        echo "Skipping installation of missing Python modules. Some functionalities might not work."
    else
        echo "User opted not to install missing Python modules. Exiting."
        exit 1
    fi
else
    echo "All required Python modules are installed."
fi


printf "\n=== OMNeT++ installation ===\n"

# Default installation path
default_install_path="$HOME/omnetpp-$version"

# Ask the user for the installation path
read -p "Enter the installation path [$default_install_path]: " install_path
install_path=${install_path:-$default_install_path}

echo "Installation path is set to $install_path"

install_omnetpp() {
  echo "Installing OMNeT++..."

  local omnetpp_download_url="https://github.com/omnetpp/omnetpp/releases/download/omnetpp-$version/omnetpp-$version-linux-x86_64.tgz"

    echo "Downloading OMNeT++ version $version... from $omnetpp_download_url"
    if ! curl -L -o "omnetpp-$version.tgz" "$omnetpp_download_url"; then
        echo "Failed to download OMNeT++. Exiting."
        return 1
    fi

    mkdir -p "$install_path"

    # Unpack OMNeT++
    echo "Unpacking OMNeT++..."
    if ! tar xvfz "omnetpp-$version.tgz" -C "$install_path" --strip-components=1; then
        echo "Failed to unpack OMNeT++. Exiting."
        return 1
    fi

    # Set environment variables
    echo "Setting environment variables..."
    if ! source "$install_path/setenv"; then
        echo "Failed to set environment variables. Exiting."
        return 1
    fi

    read -p "Do you want to configure the OMNeT++ environment automatically by adding the configuration to .profile for permanent setup? (y/n) " add_to_profile
    if [[ "$add_to_profile" =~ ^[Yy]$ ]]; then
        echo "Adding OMNeT++ environment setup to .profile..."
        echo '[ -f "'"$install_path"'/setenv" ] && source "'"$install_path"'/setenv"' >> "$HOME/.profile"
        echo "Environment variables added to .profile."
    else
        echo "Skipping the addition of environment variables to .profile."
    fi

    # Configure OMNeT++
    echo "Configuring OMNeT++..."
    cd "$install_path" || return 1
    if ! ./configure WITH_OSG=no; then
        echo "OMNeT++ configuration failed. Exiting."
        return 1
    fi

    # Build OMNeT++
    echo "Building OMNeT++..."
    if ! make -j$(nproc); then
        echo "OMNeT++ build failed. Exiting."
        return 1
    fi

    echo "OMNeT++ installation completed successfully."
}

# Check if the installation path exists and is not empty
if [ -d "$install_path" ] && [ "$(ls -A "$install_path")" ]; then
    echo "Installation path $install_path exists and is not empty."
    # Check Omnet++ version
    if "$install_path/bin/opp_run" -v 2>&1 | grep -q "Version: $version"; then
        echo "OMNeT++ version $version is already installed at $install_path. Continuing without installation"
        echo "Setting environment variables..."
        if ! source "$install_path/setenv"; then
            echo "Failed to set environment variables. Exiting."
            return 1
        fi
    else
        echo "$install_path exists but does not contain OMNeT++ version $version. Please chose another path or remove the existing installation."
        exit 1
    fi
else
    echo "Installation path $install_path does not exist or is empty. Continuing with installation."
   if ! install_omnetpp; then
     exit 1
  fi
fi



printf "\n=== Workspace setup ===\n"
default_workspace_path="$HOME/workspaces/deterministic6g_workspace"

install_d6g() (
  echo "Cloning DETERMINISTIC6G repository..."
  cd "$workspace_path" || exit 1
  if ! git clone "https://github.com/DETERMINISTIC6G/deterministic6g.git"; then
      echo "Failed cloning DETERMINISTIC6G repository. Exiting."
      exit 1
  fi
  cd "deterministic6g" || exit 1

  if ! make makefiles; then
      echo "Failed to generate makefiles. Exiting."
      exit 1
  fi
  if ! make -j$(nproc); then
      echo "Failed to build DETERMINISTIC6G. Exiting."
      exit 1
  fi
)

install_inet() (
  echo "Installing INET..."
  cd "$workspace_path" || exit 1

  if ! git clone --branch v$inet_version https://github.com/inet-framework/inet.git; then
      echo "Failed cloning INET repository. Exiting."
      exit 1
  fi

  cd "inet" || exit 1

  if ! source setenv; then
      echo "Failed to set environment variables. Exiting."
      exit 1
  fi

  if ! make makefiles; then
      echo "Failed to generate makefiles. Exiting."
      exit 1
  fi

  if ! make -j$(nproc); then
      echo "Failed to build INET. Exiting."
      exit 1
  fi
)

create_workspace() {
    read -p "Enter the workspace path [$default_workspace_path]: " workspace_path
    workspace_path=${workspace_path:-$default_workspace_path}

    if [ -d "$workspace_path" ] && [ "$(ls -A "$workspace_path")" ]; then
        echo "Workspace path $workspace_path exists and is not empty."
        echo "WARNING: Continuing will delete the contents of $workspace_path/deterministic6g and $workspace_path/inet."
        read -p "Continue? [y/n] " response
        if [[ "$response" =~ ^[Yy]$ ]]; then
            echo "Continuing with setup of $workspace_path"
        else
            echo "Exiting."
            exit 1
        fi
    else
      echo "Setting up $workspace_path"
    fi
    mkdir -p "$workspace_path"
    rm -rf "$workspace_path/deterministic6g"
    rm -rf "$workspace_path/inet"
}

fresh_setup() {
  printf "\n=== Create a new workspace ===\n"
  create_workspace
  install_inet
  install_d6g
}

existing_d6g_repo() {
  printf "\n=== Using existing DETERMINISTIC6G repository ===\n"
  echo "Using existing DETERMINISTIC6G repository at $current_script_dir"
  parent_dir=$(dirname "$current_script_dir")

  # Check if INET is installed in parent_dir
    if [ -d "$parent_dir/inet" ] && [ "$(ls -A "$parent_dir/inet")" ]; then
      echo "INET is already installed at $parent_dir/inet with the correct version, continuing"
      actual_inet_version=$("$parent_dir/inet/bin/inet_version")
      # Check version
      if [ "$actual_inet_version" == "$inet_version" ]; then
        echo "INET version $inet_version is already installed at $parent_dir/inet."
        workspace_path=$parent_dir
        return
      else
        echo "Wrong INET version $actual_inet_version in $parent_dir/inet."
        read -p "Do you want to remove the existing INET installation and install version $inet_version? [y/n] " response
        if [[ "$response" =~ ^[Yy]$ ]]; then
          rm -rf "$parent_dir/inet"
          workspace_path=$parent_dir
          install_inet
        else
          echo "Exiting."
          exit 1
        fi
      fi
    else
      echo "You now have the following options:"
      echo "1) Setup a new workspace. This will move $current_script_dir to the new workspace (Recommended)"
      echo "2) Use $parent_dir as a workspace directory and install INET there. (Only recommended if $parent_dir directory contains nothing besides the DETERMINISTIC6G repository)"
      read -p "Enter your choice [1/2]: " response
      if [[ "$response" =~ ^[1]$ ]]; then
        create_workspace
        install_inet
        # Move the DETERMINISTIC6G repository to the workspace
        cp -r "$current_script_dir" "$workspace_path/deterministic6g"
        echo "Workspace setup at $workspace_path completed successfully."
        read -p "Do you want to remove the existing DETERMINISTIC6G repository at $current_script_dir? [y/n] " response
        if [[ "$response" =~ ^[Yy]$ ]]; then
          # This line is too scary if $current_script_dir contains something important.
          # Decided to remove this for now, might add later with additional checks again.
          # rm -rf "$current_script_dir"
          echo "Please delete $current_script_dir manually"
        fi
      elif [[ "$response" =~ ^[2]$ ]]; then
        workspace_path=$parent_dir
        install_inet
      else
        echo "Invalid choice. Exiting."
        exit 1
      fi
    fi
}


# Check if the script file is part of a Git repository
check_git_repository() {
    # Get the directory of the script
    current_script_dir=$(dirname "$(realpath "$BASH_SOURCE")")

    # Check if the script's directory is inside a Git repository
    if git -C "$current_script_dir" rev-parse --is-inside-work-tree > /dev/null 2>&1; then
        return 0
    else
        return 1
    fi
}

# Check if the script file is part of a Git repository and exit if not
if ! check_git_repository; then
    fresh_setup
else
    read -p "It looks like this script is part of a Git repository, is this the version of the DETERMINISTIC6G framework you want to use? [y/n] " response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        existing_d6g_repo
    else
        echo "Continuing with a setup of a fresh workspace."
        fresh_setup
    fi
fi

setup_data_repo() (
    cd "$workspace_path" || exit 1
    if ! git clone https://github.com/DETERMINISTIC6G/deterministic6g_data; then
        echo "Failed cloning DETERMINISTIC6G data repository. Exiting."
        echo "Still continuing with the setup..."
    fi
)

printf "\n=== Data repository setup ===\n"
if [ -d "$workspace_path/deterministic6g_data" ]; then
  echo "deterministic6g_data repository found at $workspace_path/deterministic6g_data"
else
  echo "deterministic6g_data repository not found at $workspace_path/deterministic6g_data"
  read -p "Do you want to clone the deterministic6g_data repository? [y/n] " response
  if [[ "$response" =~ ^[Yy]$ ]]; then
    setup_data_repo
  else
    echo "Skipping cloning of the deterministic6g_data repository."
  fi
fi

printf "\n=== Setup completed ===\n"

# If venv is set, remind user to activate it
if [ -n "$venv_path" ]; then
  echo "In the future, please activate the venv by running 'source $venv_path/bin/activate' before opening the workspace"
fi

echo "When opening the workspace the first time make sure to import the inet and deterministic6g project from $workspace_path"
echo "You can do this by going to File -> Import -> General -> Existing Projects into Workspace"
echo "Then close the 'Welcome' tab and you're ready to go"

read -p "Do you want to open the workspace now? [y/n] " response
if [[ "$response" =~ ^[Yy]$ ]]; then
    cd "$workspace_path" || exit 1
    omnetpp -data .
    bash
fi
