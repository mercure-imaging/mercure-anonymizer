![GitHub](https://img.shields.io/github/license/mercure-imaging/mercure-anonymizer) [![Join the chat at https://gitter.im/mercure-imaging/Support](https://badges.gitter.im/mercure-imaging/Support.svg)](https://gitter.im/mercure-imaging/Support?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

# mercure-anonymizer
DICOM anonymization module for mercure

| **Note:** This anonymization module comes without any warranties. Operation of the software is solely at the user’s own risk. The authors take no responsibility for damages of any kind that may arise from usage of the software and possible distribution of data processed with the software. |
| :--- |

## Installation

Install the module on your mercure server via the Modules page of the mercure web interface. Type in the following line into the Docker tag field. mercure will then automatically download and install the module, and it will automatically install updates when they get published on Docker Hub:
```
mercureimaging/mercure-anonymizer
```
Alternatively, you can clone this GitHub repository and build the Docker container locally on your server. It is recommended to append a distinct version tag (e.g., mercureimaging/mercure-anonymizer:dev) because otherwise the version will get replaced if a newer module version gets published on Docker Hub.

If you need to modify the source code, compile the code with Qt Creator for Qt version 5.12 under Ubuntu Linux 20.04 and copy the compiled binary file into the folder /bin. Afterwards, build the Docker container.

## Module Configuration

Settings for the anonymizer module can be defined either via the Module page (global module settings) or via the Rule page (rule-dependent processing settings). Both settings are merged prior to the processing, and rule-dependent settings will overrule global module settings if defined in both places.

Settings need to be entered in JSON format.

## General and Project-Specific Settings

It is possible to provide general settings that are applied to all anonymization steps as well as project-specific settings. Project-specific settings are applied when a series (or study) has been sent to a certain Application Entity Title (AET). For example, if a series has been sent to mercure using the (receiver) AET "myproject", the "general" settings will be applied and, in addition, the settings defined in the section "myproject" will be applied (if settings are defined in both sections, project settings will overrule general settings).

The prefix "AZ_" can be added to the AET and will be ignored for the project assignment (i.e., both AETs "myproject" and "AZ_myproject" will be assigned to project "myproject"). This allows defining one global rule for all anonymization tasks by testing in the Selection Rule if "AZ_" is contained in the ReceiverAET (thus, 'AZ_' in @ReceiverAET@).

```
{
    "general": {
        ... define general settings here ...
    },
    "myproject": {
        ... define project specific settings here ...
    }
}
```

## Tag Assignment

You can specify how each DICOM tag should be processed during the anonymization by adding entires to the configurations section (either "general" or the project-specific section) in the following form:
```
Format:  "([group],[element])": "[command]"
Example: "(0010,1001)": "remove"
Example: "(0010,0010)": "set(@fake_name@)"
``` 

## Helper Macros


## Presets


