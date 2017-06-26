import qbs.base 1.0
import qbs.FileInfo

MyApp {
    Depends { name: "Qt.core" }
    Depends { name: "Qt.widgets" }

    name: "Sniper"

    files: [ "*.cpp", "*.h", "*.ui" ]

//    bundle.infoPlistFile: "Info.plist.in"

    Group {
        fileTagsFilter: bundle.isBundle ? ["bundle.content"] : ["application"]
        qbs.install: true
        qbs.installDir: mym.install_app_path
        qbs.installSourceBase: project.buildDirectory + '/' + product.destinationDirectory
    }

//    Group {
//        name: "Sniper.icns"
//        condition: qbs.targetOS.contains("osx")
//        files: [ "Sniper.icns" ]
//        qbs.install: true
//        qbs.installDir: mym.install_data_path
//    }

    Group {
        name: "sniper.png"
        condition: qbs.targetOS.contains("linux")
        files: [ "sniper.png" ]
        qbs.install: true
        qbs.installDir: "share/pixmaps"
    }

//    Group {
//        name: "sniper.desktop"
//        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("osx")
//        files: [ "sniper.desktop" ]
//        qbs.install: true
//        qbs.installDir: "share/applications"
//    }
}
