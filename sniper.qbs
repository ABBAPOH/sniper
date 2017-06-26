import qbs.base 1.0

Project {
    name: "Sniper"

    qbsSearchPaths: "qbs"

    SubProject {
        filePath: "src/src.qbs"
    }

    AutotestRunner {}
}

