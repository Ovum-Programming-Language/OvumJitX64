# Base Repository Structure

## Directory Purposes

### Root Directory
- **Project documentation** — README, build instructions, developer guides
- **Build configuration** — `CMakeLists.txt`, auxiliary CMake scripts
- **Tooling settings** — configurations for formatting, linters, Git
- **Automation scripts** — dependency installation, environment setup

### `jit/`
- **JIT compiler** — the JIT compiler for x86_64 architecture

### `.github/workflows/`
- **CI/CD configuration** — automated build and testing
- **Code quality checks** — static analysis, formatting
- **Deployment** — automated release publishing
