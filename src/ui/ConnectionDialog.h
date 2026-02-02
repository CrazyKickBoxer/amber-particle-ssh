#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTableWidget>
#include <QComboBox>
#include "../terminal/SshClient.h"

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget* parent = nullptr);

    QString host() const { return m_hostEdit->text(); }
    int port() const { return m_portSpin->value(); }
    QString user() const { return m_userEdit->text(); }
    QString password() const { return m_passEdit->text(); }
    QString keyPath() const { return m_keyOriginalPath; }
    
    QList<SshClient::PortForwardRule> forwards() const;

signals:
    void connectClicked(QString host, int port, QString user, QString password, QString keyPath);

private:
    QLineEdit* m_hostEdit;
    QSpinBox* m_portSpin;
    QLineEdit* m_userEdit;
    QLineEdit* m_passEdit;
    
    QLineEdit* m_keyEdit;
    QPushButton* m_browseKeyBtn;
    QString m_keyOriginalPath;
    
    QComboBox* m_profileCombo;
    QPushButton* m_saveProfileBtn;
    QPushButton* m_deleteProfileBtn;

    QTableWidget* m_tunnelsTable;
    QPushButton* m_addTunnelBtn;
    QPushButton* m_removeTunnelBtn;
    
    QPushButton* m_connectBtn;
    
    void loadProfiles();
    void saveProfile();
    void deleteProfile();
    void onProfileChanged(int index);
};
