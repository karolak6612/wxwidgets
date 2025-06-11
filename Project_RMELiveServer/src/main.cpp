#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QString>
#include <QDir> // For path normalization if needed

#include "qtliveserver.h" // This file will be created in the next step

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("RMELiveServer");
    QCoreApplication::setOrganizationName("RME-Qt-Project"); // Or your organization
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Remere's Map Editor - Qt Live Server");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(
        QStringList() << "p" << "port",
        QCoreApplication::translate("main", "Port for the server to listen on."),
        QCoreApplication::translate("main", "port"),
        "7171"); // Default port
    parser.addOption(portOption);

    QCommandLineOption mapOption(
        QStringList() << "m" << "map",
        QCoreApplication::translate("main", "Path to the .otbm map file to load."),
        QCoreApplication::translate("main", "mapfile"));
    parser.addOption(mapOption);

    QCommandLineOption passwordOption(
        QStringList() << "pw" << "password",
        QCoreApplication::translate("main", "Password for clients to connect (optional)."),
        QCoreApplication::translate("main", "password"),
        ""); // Default no password
    parser.addOption(passwordOption);

    QCommandLineOption dataPathOption(
        QStringList() << "d" << "datapath",
        QCoreApplication::translate("main", "Path to the game data pack (containing Tibia.dat/spr, items.otb etc.)."),
        QCoreApplication::translate("main", "path"));
    parser.addOption(dataPathOption);

    QCommandLineOption clientVersionOption(
        QStringList() << "cv" << "clientversion",
        QCoreApplication::translate("main", "Client version string for server assets (e.g., '10.98')."),
        QCoreApplication::translate("main", "version"),
        "10.98"); // Default client version for server
    parser.addOption(clientVersionOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    if (!parser.isSet(mapOption)) {
        qCritical() << "Error: Map file must be specified with --map <mapfile> option.";
        parser.showHelp(1); // Exits after showing help
    }
    if (!parser.isSet(dataPathOption)) {
        qCritical() << "Error: Data pack path must be specified with --datapath <path> option.";
        parser.showHelp(1); // Exits after showing help
    }

    QString mapFilePath = QDir::toNativeSeparators(parser.value(mapOption));
    QString dataPackPath = QDir::toNativeSeparators(parser.value(dataPathOption));
    QString serverClientVersion = parser.value(clientVersionOption);

    bool portOk;
    quint16 port = static_cast<quint16>(parser.value(portOption).toUShort(&portOk));
    if (!portOk || port == 0) {
        qCritical() << "Error: Invalid port number specified:" << parser.value(portOption);
        return 1;
    }
    QString password = parser.value(passwordOption);

    // Ensure qtliveserver.h uses appropriate namespaces if QtLiveServer is not global
    // For example: RME::server::QtLiveServer server;
    QtLiveServer server; // QtLiveServer is now defined
    if (!server.startServer(port, mapFilePath, password, dataPackPath, serverClientVersion)) {
        qCritical("Failed to start the RMELiveServer.");
        return 1;
    }

    qInfo().noquote() << QString("RMELiveServer started successfully.\n"
                               "Listening on port: %1\n"
                               "Serving map: %2\n"
                               "Password protection: %3")
                               .arg(port)
                               .arg(mapFilePath)
                               .arg(password.isEmpty() ? "No" : "Yes");

    return app.exec();
}
