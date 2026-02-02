#include "ConnectionDialog.h"
#include "../terminal/SshClient.h"
#include <QTabWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QMessageBox>
#include <QSettings>
#include <QInputDialog>
#include <QLabel>
ConnectionDialog::ConnectionDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("New Connection");
    resize(500, 400); // Larger for tabs

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Profiles UI
    QHBoxLayout* profileLayout = new QHBoxLayout();
    profileLayout->addWidget(new QLabel("Saved Connection:"));
    m_profileCombo = new QComboBox();
    profileLayout->addWidget(m_profileCombo, 1);
    
    m_saveProfileBtn = new QPushButton("Save As...");
    m_deleteProfileBtn = new QPushButton("Delete");
    
    profileLayout->addWidget(m_saveProfileBtn);
    profileLayout->addWidget(m_deleteProfileBtn);
    
    mainLayout->addLayout(profileLayout);
    
    // Separator line
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    QTabWidget* tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    
    connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectionDialog::onProfileChanged);
    connect(m_saveProfileBtn, &QPushButton::clicked, this, &ConnectionDialog::saveProfile);
    connect(m_deleteProfileBtn, &QPushButton::clicked, this, &ConnectionDialog::deleteProfile);

    // Tab 1: General
    QWidget* generalTab = new QWidget();
    QVBoxLayout* generalLayout = new QVBoxLayout(generalTab);
    
    QFormLayout* form = new QFormLayout();
    m_hostEdit = new QLineEdit("192.168.1.18");
    m_portSpin = new QSpinBox();
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(22);
    
    m_userEdit = new QLineEdit("josh");
    m_passEdit = new QLineEdit();
    m_passEdit->setEchoMode(QLineEdit::Password);
    
    m_keyEdit = new QLineEdit();
    m_browseKeyBtn = new QPushButton("Browse...");
    connect(m_browseKeyBtn, &QPushButton::clicked, this, [this](){
        QString path = QFileDialog::getOpenFileName(this, "Select Private Key");
        if (!path.isEmpty()) {
            m_keyOriginalPath = path;
            m_keyEdit->setText(path);
        }
    });

    QHBoxLayout* keyLayout = new QHBoxLayout();
    keyLayout->addWidget(m_keyEdit);
    keyLayout->addWidget(m_browseKeyBtn);

    form->addRow("Host:", m_hostEdit);
    form->addRow("Port:", m_portSpin);
    form->addRow("User:", m_userEdit);
    form->addRow("Password:", m_passEdit);
    form->addRow("Key (Optional):", keyLayout);
    
    generalLayout->addLayout(form);
    generalLayout->addStretch();
    tabWidget->addTab(generalTab, "General");

    // Tab 2: Tunnels
    QWidget* tunnelsTab = new QWidget();
    QVBoxLayout* tunnelsLayout = new QVBoxLayout(tunnelsTab);
    
    m_tunnelsTable = new QTableWidget(0, 3);
    m_tunnelsTable->setHorizontalHeaderLabels({"Local Port", "Target Host", "Target Port"});
    m_tunnelsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_addTunnelBtn = new QPushButton("Add Rule");
    m_removeTunnelBtn = new QPushButton("Remove");
    btnLayout->addWidget(m_addTunnelBtn);
    btnLayout->addWidget(m_removeTunnelBtn);
    btnLayout->addStretch();
    
    connect(m_addTunnelBtn, &QPushButton::clicked, this, [this](){
        int row = m_tunnelsTable->rowCount();
        m_tunnelsTable->insertRow(row);
        m_tunnelsTable->setItem(row, 0, new QTableWidgetItem("8080"));
        m_tunnelsTable->setItem(row, 1, new QTableWidgetItem("localhost"));
        m_tunnelsTable->setItem(row, 2, new QTableWidgetItem("80"));
    });
    
    connect(m_removeTunnelBtn, &QPushButton::clicked, this, [this](){
        int row = m_tunnelsTable->currentRow();
        if (row >= 0) m_tunnelsTable->removeRow(row);
    });
    
    tunnelsLayout->addWidget(m_tunnelsTable);
    tunnelsLayout->addLayout(btnLayout);
    tabWidget->addTab(tunnelsTab, "Tunnels");

    // Connect Button
    m_connectBtn = new QPushButton("Connect");
    m_connectBtn->setDefault(true);
    connect(m_connectBtn, &QPushButton::clicked, this, [this](){
        emit connectClicked(host(), port(), user(), password(), keyPath());
        accept();
    });
    
    mainLayout->addWidget(m_connectBtn);
    
    loadProfiles();
}

void ConnectionDialog::loadProfiles()
{
    m_profileCombo->blockSignals(true);
    m_profileCombo->clear();
    m_profileCombo->addItem("- Select -");
    
    QSettings settings("AmberSSH", "Profiles");
    settings.beginGroup("Saved");
    QStringList profiles = settings.childGroups();
    m_profileCombo->addItems(profiles);
    settings.endGroup();
    
    m_profileCombo->blockSignals(false);
}

void ConnectionDialog::saveProfile()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Save Connection", "Profile Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    
    QSettings settings("AmberSSH", "Profiles");
    settings.beginGroup("Saved");
    settings.beginGroup(name);
    settings.setValue("host", host());
    settings.setValue("port", port());
    settings.setValue("user", user());
    settings.setValue("password", password()); // Unsafe? User's choice.
    settings.setValue("keyPath", keyPath());
    settings.endGroup();
    settings.endGroup();
    
    loadProfiles();
    m_profileCombo->setCurrentText(name);
}

void ConnectionDialog::deleteProfile()
{
    QString name = m_profileCombo->currentText();
    if (name == "- Select -") return;
    
    if (QMessageBox::question(this, "Delete Profile", "Delete '" + name + "'?") == QMessageBox::Yes) {
        QSettings settings("AmberSSH", "Profiles");
        settings.beginGroup("Saved");
        settings.remove(name);
        settings.endGroup();
        loadProfiles();
    }
}

void ConnectionDialog::onProfileChanged(int index)
{
    QString name = m_profileCombo->currentText();
    if (name == "- Select -" || name.isEmpty()) return;
    
    QSettings settings("AmberSSH", "Profiles");
    settings.beginGroup("Saved");
    settings.beginGroup(name);
    
    m_hostEdit->setText(settings.value("host").toString());
    m_portSpin->setValue(settings.value("port", 22).toInt());
    m_userEdit->setText(settings.value("user").toString());
    m_passEdit->setText(settings.value("password").toString());
    
    QString key = settings.value("keyPath").toString();
    if (!key.isEmpty()) {
        m_keyOriginalPath = key;
        m_keyEdit->setText(key);
    }
    
    settings.endGroup();
    settings.endGroup();
}

QList<SshClient::PortForwardRule> ConnectionDialog::forwards() const
{
    QList<SshClient::PortForwardRule> rules;
    for (int i=0; i<m_tunnelsTable->rowCount(); ++i) {
        SshClient::PortForwardRule rule;
        rule.type = SshClient::PortForwardRule::Local; // Support local only for now
        rule.bindPort = m_tunnelsTable->item(i, 0)->text().toInt();
        rule.targetHost = m_tunnelsTable->item(i, 1)->text();
        rule.targetPort = m_tunnelsTable->item(i, 2)->text().toInt();
        rules.append(rule);
    }
    return rules;
}
