#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "building.h"
#include "add1.h"
#include "longestpair.h"
#include <string>
#include <QTimer>
#include <QtDebug>
#include "shygame.h"
#include "fib.h"
#include "findfactorial.h"
#include "largefactorial.h"
#include "people.h"
#include "easycity2.h"
#include "longestshorestdisstance.h"
#include "judgewindow.h"
#include <time.h>
#include <QString>
using namespace std;
Building::Building()
{

    //JudgeWindow judge;
    judge.show();
    judge.setSeed(0);
    int n=judge.getConditionNum(); //get People data according variable n
    //create people
    query.exec("drop schema if exists Course6");
    query.exec("create schema if not exists Course6");
    query.exec("use Course6");
    query.exec("drop table if exists peoplelist");
    query.exec("create table if not exists peoplelist (id char(8),Nowfloor int,Destination int,Number int,PRIMARY KEY(id))");
    query.exec("load data infile 'C:/ProgramData/MySQL/MySQL Server 8.0/Uploads/data.csv' into table peoplelist fields terminated by ',' lines terminated by '\n' ignore 1 rows");

    query.exec("select n.* from (select * from peoplelist limit "+QString::number(n-1)+",1) as s,peoplelist as n where left(n.id,5)=left(s.id,5)");
    for(int i=0;i<10;i++){
        people[i] = new People;
        //cout<<query.value(0).toInt()<<endl;
        people[i]->setPeople(query);
    }
    //floor
    floor[0] = new Floor(new Add1());
    floor[1] = new Floor(new Prime());
    floor[2] = new Floor(new LongestPair());
    floor[3] = new Floor(new Shygame());
    floor[4] = new Floor(new Fib());
    floor[5] = new Floor(new FindFactorial());
    floor[6] = new Floor();
    floor[7] = new Floor(new LargeFactorial());
    floor[8] = new Floor();
    floor[9] = new Floor(new EasyCity2());
    floor[10] = new Floor(new LongestShorestDisstance());


}
void Building::run(int question)
{
    //0.txt
    string s = judge.getData(question,0);
    data.testdata1 = s;

    string s2 = floor[question]->p->solve(s);


    data.submit1 = s2;
    cout<<"sdfadfas"<<s2<<endl;
    bool correct = judge.submitData(s2);
    data.correct1 = correct;
    data.spendtime1 = judge.getSpendTime();

}

