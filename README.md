# Soul Engine

![GitHub](https://img.shields.io/github/license/Synodic-Software/Soul-Engine.svg)
![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)


Soul Engine is a real-time visualization engine built on the back of CUDA and Vulkan. With a focus on lightweight parallelism that can be leveraged for expensive techniques, Soul Engine provides a platform for path tracing, finite element physics, and general simulation.

For more information, visit our [Wiki](https://github.com/Synodic-Software/Soul-Engine/wiki)
- - - -
## **Warning**
Soul Engine is undergoing a large user-facing refactor and is not stable. Incomplete public features will be removed in preparation for future Semantic Versioning.

In the meantime expect to see the following features make their way in:

* Subsystem modularity
* Testing framework
* Continuous integration
* Simplified manual installation
* Automated C++/Python dependency installation
* Dynamic and static plugins
* Directory tree cleanup

Once Soul Engine hits 1.0, codebase stability will become a priority. Thank you for your patience. 

- - - -

![Tree Model](Resources/Images/Tree.png)


# Getting Started

1. Install the required manual dependencies
    * [Python 3.8](https://www.python.org/downloads/)
    * [Poetry](https://poetry.eustace.io/docs/#installation) 
        * Install within the global pip environement - `pip install poetry`
        * Install for your user - `curl -sSL https://raw.githubusercontent.com/sdispater/poetry/master/get-poetry.py | python`

1. Clone `Soul Engine`
    * `git clone https://github.com/Synodic-Software/Soul-Engine.git`

1. Install the dependencies and setup the tooling 
    * `poetry install`

1. Build the engine 
    * `poetry build`

1. Run `Soul Engine`
    * `soul info`

For a detailed installation process visit our Wiki's [Getting Started](https://github.com/Synodic-Software/Soul-Engine/wiki/Getting-Started)