import qbs 1.0

Module {
    property string app_target: qbs.targetOS.contains("osx") ? "Sniper" : "sniper"

    property string install_app_path: {
        if (qbs.targetOS.contains("osx"))
            return ".";
        else if (qbs.targetOS.contains("windows"))
            return ".";
        else
            return "bin";
    }

    property string install_binary_path: {
        if (qbs.targetOS.contains("osx"))
            return app_target + ".app/Contents/MacOS"
        else
            return install_app_path
    }

    property string lib_suffix: ""

    property string install_library_path: {
        if (qbs.targetOS.contains("osx"))
            return app_target + ".app/Contents/Frameworks"
        else if (qbs.targetOS.contains("windows"))
            return install_app_path
        else
            return "lib" + lib_suffix + "/" + app_target
    }

    property string install_plugin_path: {
        if (qbs.targetOS.contains("osx"))
            return app_target + ".app/Contents/PlugIns"
        else
            return install_library_path + "/plugins"
    }

    property string install_data_path: {
        if (qbs.targetOS.contains("osx"))
            return app_target + ".app/Contents/Resources"
        else
            return "share/" + app_target
    }

    property stringList includePaths: []
    property stringList libraryPaths: []

    property stringList commonFlags: []
    property stringList cFlags: commonFlags
    property stringList cxxFlags: commonFlags
    property stringList linkFlags: commonFlags

    property string buildType: "dynamic"
    property bool staticBuild: buildType == "static"
    property bool frameworksBuild: qbs.targetOS.contains("osx") && buildType == "frameworks"
    property bool checkBuildType: {
        if (buildType != "static" && buildType != "dynamic" && buildType != "frameworks")
            throw "Invalid build type: should be one of [static, dynamic, frameworks]";
        return true;
    }
}
