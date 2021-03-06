#ifndef JUDGEWINDOW_H
#define JUDGEWINDOW_H

#include <QElapsedTimer>
#include <string>
#include <iostream>
#include <QWidget>
#include <QApplication>
#include <QLineEdit>
#include <QCheckBox>
#include <QSqlQuery>
#include <QLabel>
#include "people.h"
using namespace std;
namespace Ui {
class JudgeWindow;
}

class JudgeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit JudgeWindow(QWidget *parent = nullptr);
    ~JudgeWindow();
    string getData(int floor,int b,int& datatimes);//input 0-26
    bool submitData(int floor,string ans);
    void setSeed(int seed){srand(seed);}
    qint64 getSpendTime(){return costtime;}
    int getConditionNum();//return 1-300
    void getDistance(int d){distance = d;}
    void scheduleEnd();
    void getInitialCondition(People *people[]);
    void display(int);
    long long int getScore(){return score;}

    QLineEdit showline[27][4];
    QCheckBox boxlist[27][1];
    QLabel floorLabel[27][1];
private slots:
    void on_export_to_database_clicked();

private:
    long long int arr[27][4];
    Ui::JudgeWindow *ui;
    qint64 costtime;
    int distance;
    QSqlQuery query;
    QElapsedTimer timer;
    string answer;
    long long int score;
    int floordatatimes[27];
    int floornextdata[27];
    int pass[27];
    QSqlDatabase database2;


};

#endif // JUDGEWINDOW_H
