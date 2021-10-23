#pragma once

#include <QObject>

class Client : public QObject
{
    Q_OBJECT

public:
    void init();
};
