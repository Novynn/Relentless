#include "clientcore.h"
#include "client.h"

ClientCore::ClientCore(Core *parent) :
    CoreObject(parent){
}

void ClientCore::load(){
    clientConfigPath = getClientConfigPath();

    info("ClientCore initialized (@" + QString::number(core()->uptime()) + "ms)");

    maxClients = 99;
    multiClientMode = false;
}

void ClientCore::loadClients(){
    QString clientConfigPath = core()->getClientConfigPath();
    QDir clientConfigDir(clientConfigPath);
    print("Searching [" + clientConfigDir.path().mid(QDir::currentPath().length()) + "] for client configuration files...");
    QStringList filter;
    filter.append("*.conf");
    filter.append("*.ini");
    filter.append("*.cfg");
    foreach(QString file, clientConfigDir.entryList(filter)){
        bool r = newClient(file);
        if (!r && clients.count() >= maxClients)
            break;
    }
    print("Configuration files loaded. We have [" + QString::number(clients.count()) + "] client/s.");

    if (clients.count() > 1){
        info("> Multi-client mode enabled.");
        multiClientMode = true;
    }
    info("Finished loading clients (@" + QString::number(core()->uptime()) + "ms)");
}

void ClientCore::clientOutput(Client* client, QString message, MessageType type){
    QString identifier = client->getIdentifier();
    core()->printMessage(message, MessageOrigin(alias() + "\\" + identifier), type);

    QVariantHash data;
    data.insert("message", message);
    data.insert("type", QVariant::fromValue<MessageType>(type));
    emitEvent(client, "output", data);
}

QHash<QString, QString> ClientCore::getBlocks(const QString fileName, const QString parentName){
    QHash<QString, QString> blocks;
    QFile file(core()->getClientConfigPath() + fileName);
    if (!file.exists()) return blocks;
    if (!file.open(QFile::ReadOnly)) return blocks;
    QString data = file.readAll();
    file.close();

    bool insideBlock = false;
    QString currentBlock;
    QString blockContents;

    int count = 0;
    foreach(QString line, data.split("\n")){
        line = line.simplified();
        count++;
        if (!line.contains("#@(")){
            if (insideBlock)
                blockContents.append(line + "\n");
        }

        int j = line.indexOf("#@(");

        if(j != 0) continue;

        //qDebug() << "  line " << count << ": " << line;

        QString innerBlock = line.mid(3);
        innerBlock = innerBlock.left(innerBlock.length() - 1);

        QString type;
        QStringList arguments;
        QStringList parts = innerBlock.split(" ", QString::SkipEmptyParts);
        if (parts.count() > 0){
            type = parts.takeFirst().trimmed();
            QString currentString;
            bool insideString = false;
            while(!parts.isEmpty()){
                QString stringBlock = parts.takeFirst();
                if (!insideString){
                    if (stringBlock.startsWith("\"")){
                        if (stringBlock.endsWith("\"")){
                            arguments.append(stringBlock.mid(1, stringBlock.length() - 2));
                        }
                        else {
                            insideString = true;
                            currentString = stringBlock.mid(1) + " ";
                        }
                    }
                    else
                        arguments.append(stringBlock);
                }
                else {
                    if (stringBlock.endsWith("\"")){
                        insideString = false;
                        arguments.append(currentString + stringBlock.mid(0, stringBlock.length() - 1));
                    }
                    else
                        currentString.append(stringBlock + " ");
                }
            }

            // Imports blocks from the specified file
            if (type == "import"){
                QString newFile = arguments.first();
                if (newFile == parentName || newFile == fileName ){
                    error("Infinite loop detected [in " + fileName + " while trying to include " + newFile + " (parent: " + parentName + ")].");
                    break;
                }
                QHash<QString, QString> moreBlocks = getBlocks(newFile, fileName);
                blocks.unite(moreBlocks);
            }
            else if (type == "insert"){
                // Ignore
            }
            // Declares a block
            else if (type == "block"){
                if (insideBlock){
                    //qDebug() << "BLOCKS CANNOT BE NESTED";
                    continue;
                }
                //qDebug() << "STARTING BLOCK " << arguments.first();
                insideBlock = true;
                currentBlock = arguments.first();
            }
            // Ends a blocks declaration
            else if (type == "endblock"){
                if (!insideBlock){
                    //qDebug() << "STRAY ENDBLOCK";
                    continue;
                }
                //qDebug() << "ENDING BLOCK " << currentBlock;
                blocks.insert(currentBlock, blockContents);

                insideBlock = false;
                currentBlock.clear();
                blockContents.clear();
            }
            else {
                qDebug() << "Unrecognized type: " << type;
            }

            j++;
        }
    }

    return blocks;
}

QString ClientCore::parseConfigFile(const QString configFile){
    QHash<QString, QString> blocks = getBlocks(configFile);
    QFile file(core()->getClientConfigPath() + configFile);
    if (!file.exists()) return QString();
    if (!file.open(QFile::ReadOnly)) return QString();
    QString data = file.readAll();
    QString newData;
    file.close();

    bool insideBlock = false;
    int count = 0;
    foreach(QString line, data.split("\n")){
        line = line.simplified();
        count++;
        if (!line.contains("#@(")){
            if (!insideBlock){
                newData.append(line + "\n");
            }
            continue;
        }

        int j = 0;
        while((j = line.indexOf("#@(", j)) != -1){
            int start = line.indexOf("#@(", j);
            int end = line.indexOf(")", j);

            QString block = line.mid(start, (end - start) + 1);
            QString innerBlock = block.mid(3);
            innerBlock = innerBlock.left(innerBlock.length() - 1);

            QString type, content;
            QStringList arguments;
            QStringList parts = innerBlock.split(" ", QString::SkipEmptyParts);
            if (parts.count() > 0){
                type = parts.takeFirst().trimmed();
                QString currentString;
                bool insideString = false;
                while(!parts.isEmpty()){
                    QString stringBlock = parts.takeFirst();
                    if (!insideString){
                        if (stringBlock.startsWith("\"")){
                            if (stringBlock.endsWith("\"")){
                                arguments.append(stringBlock.mid(1, stringBlock.length() - 2));
                            }
                            else {
                                insideString = true;
                                currentString = stringBlock.mid(1) + " ";
                            }
                        }
                        else
                            arguments.append(stringBlock);
                    }
                    else {
                        if (stringBlock.endsWith("\"")){
                            insideString = false;
                            arguments.append(currentString + stringBlock.mid(0, stringBlock.length() - 1));
                        }
                        else
                            currentString.append(stringBlock + " ");
                    }
                }

                // Imports blocks from the specified file
                if (type == "import"){
                    // Ignore
                }
                else if (type == "insert"){
                    QString blockName = arguments.first();
                    QString blockData = blocks.value(blockName, "");
                    if (!blockData.isEmpty())
                        newData.append(blockData);
                }
                // Declares a block
                else if (type == "block"){
                    if (insideBlock)
                        continue;
                    insideBlock = true;
                }
                // Ends a blocks declaration
                else if (type == "endblock"){
                    if (!insideBlock)
                        continue;
                    insideBlock = false;
                }
                else {
                    qDebug() << "Unrecognized type: " << type;
                }
            }
            j++;
        }
    }
    QString matcher = QDir::tempPath() + "/" + QFileInfo(file).fileName() + ".~TEMP(XXXXXX)";
    QByteArray bytes = newData.toLatin1();
    QTemporaryFile* newFile = new QTemporaryFile(matcher, this);
    newFile->setAutoRemove(true);
    newFile->open();
    newFile->write(bytes);
    newFile->close();
    return newFile->fileName();
}

bool ClientCore::newClient(const QString configFile){
    //info("Start parsing (@" + QString::number(core()->uptime()) + "ms)");
    QString path = parseConfigFile(configFile);
    //info("End parsing (@" + QString::number(core()->uptime()) + "ms)");
    QSettings* settings = new QSettings(path, QSettings::IniFormat);
    if (settings->status() != QSettings::NoError){
        QString reason = (settings->status() == QSettings::FormatError) ? "The file's format is invalid." : "Could not access the file." ;
        error(QString("Could not load [%1] due to [%2]").arg(configFile).arg(reason));
        delete settings;
        return false;
    }
    settings->beginGroup("Main");
    QString identifier = settings->value("identifier", "").toString();
    QString username = settings->value("username", "").toString();
    settings->endGroup();
    uint attempts = 0;
    if (identifier.isEmpty())
        identifier = username;
    while(identifier.isEmpty() || clients.contains(identifier)){
        if (attempts >= 5) break;
        warning("Client [" + configFile + "] has an invalid or duplicated identifier. Generating a temporary one now...");
        QString newIdentity(QCryptographicHash::hash((QString::number(core()->uptime()) + configFile).toUtf8(), QCryptographicHash::Md5).toHex().left(6));
        identifier = newIdentity;
        warning("Client [" + configFile + "] now has the identifier of [" + identifier + "]");
        ++attempts;
    }
    if (attempts >= 5){
        error("Could not set the identifier for [" + configFile + "] within [" + QString::number(attempts) + "] attempts.");
        delete settings;
        return false;
    }
    if (clients.count() >= maxClients){
        error("Maximum clients amount reached.");
        delete settings;
        return false;
    }
    //info("Creating client... (@" + QString::number(core()->uptime()) + "ms)");
    Client* client = new Client(identifier, this, settings);
    if (!client->load()){
        warning("Configuration of [" + configFile + "] is invalid.");
        client->unload();
        removeClient(client);
        return false;
    }
    clients.insert(client->getIdentifier(), client);
    info("Client created! (@" + QString::number(core()->uptime()) + "ms)");

    emitEvent(client, "created");

    return true;
}

void ClientCore::emitEvent(Client* client, QString event, QVariantHash data) {
    QVariantHash log;
    log.insert("client", client->getIdentifier());
    log.insert("event", event);
    log.insert("data", data);
    eventLog.enqueue(log);

    emit clientEvent(event, client->getIdentifier(), data);
}

void ClientCore::clientRequest(QString event, QString id, QVariantHash data)
{
    Client* client = getClient(id);
    if (!client) return;
    if (event.toLower() == "message") {
        QString message = data.value("message", "").toString();
        client->command(message);
        if (message.left(1) != "/"){
            QVariantHash callback;
            callback.insert("username", client->username());
            callback.insert("text", message);
            emitEvent(client, "chat_usertalk", callback);
        }
    }
    if (event.toLower() == "destroy") {
        info("Recieved a request to destroy the client: " + id);
        removeClient(client);
    }
    if (event.toLower() == "connect") {
        info("Recieved a request to connect the client: " + id);
        client->connectClient();
    }
    if (event.toLower() == "disconnect") {
        info("Recieved a request to disconnect the client: " + id);
        client->disconnectClient();
    }
}

Client* ClientCore::getClient(const QString clientAlias){
    return clients.value(clientAlias);
}

QStringList ClientCore::getClientAliases(){
    return clients.keys();
}

bool ClientCore::hasClient(const QString clientAlias){
    Client* c = clients.value(clientAlias);
    return c;
}

bool ClientCore::hasClient(Client* client){
    return hasClient(client->getIdentifier());
}

bool ClientCore::removeClient(const QString clientAlias){
    Client* c = clients.value(clientAlias);
    if (c){
        return removeClient(c);
    }
    return false;
}

bool ClientCore::removeClient(Client* client){
    if (client) {
        QString alias = client->getIdentifier();
        emitEvent(client, "destroyed");
        client->unload();
        delete client;
        clients.remove(alias);
    }
    client = 0;

    return true;
}

void ClientCore::unloadClients(){

}

void ClientCore::unloadClient(Client *client){
    removeClient(client);
}

void ClientCore::sendConnectSignal(){
    foreach(Client* client, clients.values()){
        client->connectClient();
    }
}

void ClientCore::sendDisconnectSignal(){
    foreach(Client* client, clients.values()){
        client->disconnectClient();
    }
}

