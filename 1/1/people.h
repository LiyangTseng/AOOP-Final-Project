#ifndef PEOPLE_H
#define PEOPLE_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <string>
#include <iostream>
#include <QString>
#include <QSqlQuery>
using namespace std;


class People
{
public:
    People();
    void setPeople(QSqlQuery &query);
    int getPeopleNum();
    int getPeopleDes();
//private:
    QString id;
    int Nowfloor;
    int Destination;
    int Number;
//private:
//    QSqlQuery query;
};

#endif // PEOPLE_H
