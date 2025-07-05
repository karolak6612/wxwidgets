bool BrushMaterialEditorDialog::saveWallBrushesToXml()
{
    if (!ensureXmlDirectoryExists()) {
        QMessageBox::warning(this, "Error", "Could not create XML directory.");
        return false;
    }
    
    QString wallsPath = getXmlFilePath("walls.xml");
    
    QDomDocument doc;
    QDomElement root;
    
    // Try to load existing file
    QFile existingFile(wallsPath);
    if (existingFile.exists() && existingFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (!doc.setContent(&existingFile)) {
            existingFile.close();
            QMessageBox::warning(this, "Error", "Could not parse existing walls.xml file.");
            return false;
        }
        existingFile.close();
        root = doc.documentElement();
    } else {
        // Create new document
        QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\"");
        doc.appendChild(xmlDeclaration);
        root = doc.createElement("materials");
        doc.appendChild(root);
    }
    
    // TODO: Save actual wall brush data
    
    // Write to file
    QFile file(wallsPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open walls.xml for writing.");
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toString(2);
    file.close();
    
    return true;
}

bool BrushMaterialEditorDialog::saveDoodadBrushesToXml()
{
    if (!ensureXmlDirectoryExists()) {
        QMessageBox::warning(this, "Error", "Could not create XML directory.");
        return false;
    }
    
    QString doodadsPath = getXmlFilePath("doodads.xml");
    
    QDomDocument doc;
    QDomElement root;
    
    // Try to load existing file
    QFile existingFile(doodadsPath);
    if (existingFile.exists() && existingFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (!doc.setContent(&existingFile)) {
            existingFile.close();
            QMessageBox::warning(this, "Error", "Could not parse existing doodads.xml file.");
            return false;
        }
        existingFile.close();
        root = doc.documentElement();
    } else {
        // Create new document
        QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\"");
        doc.appendChild(xmlDeclaration);
        root = doc.createElement("materials");
        doc.appendChild(root);
    }
    
    // TODO: Save actual doodad brush data
    
    // Write to file
    QFile file(doodadsPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open doodads.xml for writing.");
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toString(2);
    file.close();
    
    return true;
}