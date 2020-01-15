#include "judgewindow.h"
#include "ui_judgewindow.h"
#include <QSqlError>
#include <fstream>

#define FIRSTRUN 1

JudgeWindow::JudgeWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::JudgeWindow),
    globalseed(0)
{

    this->setWindowTitle("FINAL");
    ui->setupUi(this);

    initializeDB();

    initializeGUI();

    initializeSystem();
}

//不用更改
void JudgeWindow::initializeGUI(){
    ui->gridLayout->addWidget(new QLabel("   "),0,0);
    ui->gridLayout->addWidget(new QLabel("Ele. place"),0,1);
    ui->gridLayout->addWidget(new QLabel("Leaving"),0,2);
    ui->gridLayout->addWidget(new QLabel("Arriving"),0,3);
    ui->gridLayout->addWidget(new QLabel("Correct"),0,4);
    ui->gridLayout->addWidget(new QLabel("Score"),0,5);


    for(int i=0;i<27;i++){
        ui->gridLayout->addWidget(new QLabel(QString::number(i+1)), i+1, 0);
        eleplace[i] = new QLabel();
        ui->gridLayout->addWidget(eleplace[i], i+1, 1);
        for(int j=0;j<4;j++){
            showline[i][j] = new QLineEdit();
            if(j != 2)
                showline[i][j]->setAlignment(Qt::AlignHCenter);
            else
                showline[i][j]->setAlignment(Qt::AlignRight);
            ui->gridLayout->addWidget(showline[i][j],i+1,j+2);
        }
        giveupbox[i] = new QCheckBox();
        giveupbox[i]->setCheckState(Qt::Checked);   //testing
        ui->gridLayout->addWidget(giveupbox[i], i+1, 6);
    }
    ui->gridLayout->setColumnStretch(2, 1);
    ui->gridLayout->setColumnStretch(3, 1);
    eleplace[0]->setText("I'm here");

}

void JudgeWindow::initializeDB(){
    db=QSqlDatabase::addDatabase("QMYSQL", "mysql");
    // connection name "mysql" represent variable "db", 不用更改

    //改成你自己的UserName & Password
    db.setHostName("localhost");
    db.setUserName("root");
    db.setPassword("123456789");
    db.setPort(3306);
    (db.open())?qDebug()<<"JUDGE DB OK":qDebug()<<"JUDGE DB NOT OK";

    QSqlQuery query(db);
#if FIRSTRUN
    query.exec("drop database if exists final;");
    query.exec("create database if not exists final");
    query.exec("use final");
    query.exec("create table if not exists testdata( \
               id char(8), \
               groupnum int, \
               ques text, \
               ans text, \
               primary key(id) \
               );");
    //path改成你的檔案路徑
    QString path = "C:/ProgramData/MySQL/MySQL Server 8.0/Uploads/testdata_1225.csv";
    query.exec("load data infile '" + path +"' \
               into table testdata \
               fields terminated by ',' \
               enclosed by '\"' \
               lines terminated by '\r\n' \
               ignore 1 rows");

    query.exec("create table if not exists peoplelist( \
               id char(8), \
               nowfloor int, \
               destination int, \
               num int, \
               primary key(id) \
               )");

    //path改成你的檔案路徑
    path = "C:/ProgramData/MySQL/MySQL Server 8.0/Uploads/simple_initial_condition.csv";
    query.exec("load data infile '" + path + "' \
               into table peoplelist \
               fields terminated by ',' \
               enclosed by '\"' \
               lines terminated by '\r\n' \
               ignore 1 rows ");

#endif
}

void JudgeWindow::initializeSystem(){

    random.seed(globalseed);

    QSqlQuery query(db);
    QSqlQuery query2(db);
    QString command;
    query.exec("use final");
    query2.exec("use final");
    query.exec("select count(*) from peoplelist");
    query.next();
    int conditionCount = query.value(0).toInt()/FLOORNUMBER;

    int now;
    int des;
    int num;
    bool existflag = false;

    command.sprintf("select exists(select * from peoplelist where id regexp \'^99999\')");
    query.exec(command);
    query.next();

    //是否當了, 此為否
    if(query.value(0).toInt() == 0){

        query.exec("create table if not exists testdataresult( \
                   indexfloor int, \
                   correctness int, \
                   costtime bigint, \
                   seed bigint \
                   )");

        command.sprintf("select exists(select * from testdataresult)");
        query.exec(command);
        query.next();

        //是否為第一次跑, 此為否
        if(query.value(0).toInt() == 1){
            int floorindex, correctness;
            qint64 costtime;
            quint32 seed;
            query.exec("select * from testdataresult");
            while(query.next()){
                floorindex = query.value(0).toInt();
                correctness = query.value(1).toInt();
                costtime = query.value(2).toLongLong();
                seed = query.value(3).toUInt();
                performap[floorindex][seed] = Performance(correctness, costtime);
            }

            query.exec("drop table testdataresult");
            query.exec("create table if not exists testdataresult( \
                       indexfloor int, \
                       correctness int, \
                       costtime bigint, \
                       seed bigint \
                       )");
        }
        else{
        }

        //開始比賽, 建立恢復所需資訊
        query.exec("create table if not exists totalresultrecover( \
                   pathlen int, \
                   overweight int, \
                   peoplepass int \
                   )");
        query.exec("insert into totalresultrecover values(0, 0, 0)");
        query.exec("create table if not exists floorinfrecover( \
                   indexfloor int, \
                   totalcorrect int, \
                   totalque int, \
                   totalscore int, \
                   totalcosttime bigint, \
                   currentseed bigint \
                   )");

        ui->overWeightLabel->setText("Overweight : No");
        ui->TotalPathLabel->setText("Path length : 0");
        ui->TotalScoreLabel->setText("Total score : 0");


        conditionNum=random.bounded(conditionCount)+1;
        command.sprintf("select * from peoplelist where id like '%05d-%%'",conditionNum);
        existflag = false;


    }
    //上次當掉, 開始恢復的動作
    else{

        command.sprintf("select * from totalresultrecover");
        query.exec(command);
        query.next();
        distance = query.value(0).toInt();
        ui->TotalPathLabel->setText("Path length : " + QString::number(distance));
        if(query.value(1).toInt() == 1)
            ui->overWeightLabel->setText("Overweight : Yes");
        else
            ui->overWeightLabel->setText("Overweight : No");
        totalpeople = query.value(2).toInt();

        int indexfloor, totalcorrect, totalque, totalscore;
        long long totalcosttime;
        uint currentseed;
        query.exec("select * from floorinfrecover");
        while(query.next()){
            int i=0;
            indexfloor = query.value(i++).toInt();
            totalcorrect = query.value(i++).toInt();
            totalque = query.value(i++).toInt();
            totalscore = query.value(i++).toInt();
            totalcosttime = query.value(i++).toLongLong();
            currentseed = query.value(i++).toUInt();
            correct[indexfloor]=totalcorrect;
            quesnum[indexfloor]=totalque;
            costlist[indexfloor]=totalcosttime;
            floorscore[indexfloor]=totalscore;
            floorseed[indexfloor] = currentseed;
            this->totalscore += totalscore;
            if(currentseed == 1024)
                giveupbox[indexfloor]->setCheckState(Qt::Checked);

            this->showline[indexfloor][2]->setText(QString::number(correct[indexfloor]));
            this->showline[indexfloor][3]->setText(QString::number(floorscore[indexfloor]));
        }
        ui->TotalScoreLabel->setText("Total score : " + QString::number(this->totalscore));
        ui->TotalPathLabel->setText("Path length : "+QString::number(distance));
        conditionNum = 99999;
        command.sprintf("select * from peoplelist where id regexp '^%05d'", conditionNum);
        existflag = true;

    }
    //qDebug() << command;
    query.exec(command);
    while(query.next()){
        now=query.value(1).toInt();
        des=query.value(2).toInt();
        num=query.value(3).toInt();
        //當掉解決
        if(!existflag){
            command.sprintf("insert into peoplelist values(\'%s-%02d\',%d,%d,%d)","99999",now,now,des,num);
            query2.exec(command);
        }
        waitingpeople[now-1]=num;
        arrivingpeople[des-1]=num;
        totalpeople += num;
        this->showline[des-1][1]->setText(QString::number(num));
        this->showline[now-1][0]->setText(QString::number(num));
    }

    ui->TotalPeopleLabel->setText("Total people : " + QString::number(totalpeople));

    for(int i=0;i<FLOORNUMBER;i++){

        command.sprintf("select count(id) from testdata where id like '%02d-%%'",i+1);
        query.exec(command);
        query.next();
        testdatacount[i] = query.value(0).toUInt();

        if(!existflag){
            do{
                floorseed[i]=random.bounded(UINT_MAX);
            }while(floorseed[i] == 1024);
            command.sprintf("insert into floorinfrecover values(%d, 0, 0, 0, 0, %u)", i, floorseed[i]);
        }
        //qDebug() << command;
        query.exec(command);

        if(i < 21)
            runtimecount[i] = 50;
        else
            runtimecount[i] = 1;

    }
}

string JudgeWindow::getData(int index, int b, int &times){

    if(index<0||index>=FLOORNUMBER){
        qDebug()<<__func__<<"FLOOR ERROR";
        return "";
    }

    QSqlQuery query(db);
    query.exec("use final");
    QString command;

    if(b==0){
        arrivingpeople[index]--;
        elevactorPeople--;
        showline[index][1]->setText(QString::number(arrivingpeople[index]));
    }
    else{
        waitingpeople[index]--;
        elevactorPeople++;
        command = "update peoplelist set num = num-1 where id = \'99999-" + (QString::number(index+1)).rightJustified(2, '0') + "\'";
        query.exec(command);
        command.sprintf("update totalresultrecover set peoplepass = peoplepass+1");
        query.exec(command);
        showline[index][0]->setText(QString::number(waitingpeople[index]));
    }

    if(elevactorPeople>ELEVATORLIMIT||elevactorPeople<0){
        distance += (elevactorPeople - ELEVATORLIMIT) * 60;
        //punishment
        ui->overWeightLabel->setText("Overweight : Yes");
        query.exec("update totalresultrecover set overweight = 1");
    }

    distance+=abs(lastfloor-index);
    //qDebug() << (index+1);
    ui->TotalPathLabel->setText("Path length : "+QString::number(distance));
    command.sprintf("update totalresultrecover set pathlen = %d", distance + index);
    query.exec(command);

    int floor=index+1;
    random.seed(floorseed[index]);

    uint select = floorseed[index]%(testdatacount[index]);
    command.sprintf("update floorinfrecover set currentseed = 1024 where indexfloor = %d", index);
    query.exec(command);

    command.sprintf("select * from testdata where id = '%02d-%05d'",floor,select);
    query.exec(command);
    if(!query.next()){qDebug()<<__func__<<"select error"<<command;}

    ques=query.value(2).toString().toStdString();
    ans=query.value(3).toString().toStdString();

    eleplace[lastfloor]->setText("");
    eleplace[index]->setText("I'm here");
    this->repaint();
    lastfloor=index;

    quesnum[index]++;

    times = runtimecount[index];

    if(this->giveupbox[index]->isChecked())
        ques = "GIVEUP";

    timer.start();
    return ques;
}

bool JudgeWindow::submitData(string ans){

    if(lastfloor < 21)
        costtime=timer.nsecsElapsed()/runtimecount[lastfloor];
    else
        costtime = 0;

    bool correctans = false;
    if(this->ans.compare(ans)==0){
        costlist[lastfloor]+=costtime;
        correct[lastfloor]++;
        correctans = true;

        if((lastfloor >= 21) || (performap[lastfloor].find(floorseed[lastfloor]) == performap[lastfloor].end()) || performap[lastfloor][floorseed[lastfloor]].correctness == 0){
            floorscore[lastfloor] += 100;
            totalscore += 100;
        }
        else{
            long long diftime = abs(costtime - performap[lastfloor][floorseed[lastfloor]].costtime);
            long long maxtime = max(costtime, performap[lastfloor][floorseed[lastfloor]].costtime);

            double difratio = (static_cast<double>(diftime))/maxtime;
            if(difratio < 0.25){
                floorscore[lastfloor] += 100;
                totalscore += 100;
            }
        }
    }

    QSqlQuery query(db);
    query.exec("use final");
    QString command;
    if(correctans)
        command.sprintf("insert into testdataresult values(%d, 1, %lld, %u)", lastfloor, costtime, floorseed[lastfloor]);
    else
        command.sprintf("insert into testdataresult values(%d, 0, 0, %u)", lastfloor, floorseed[lastfloor]);
    query.exec(command);
    this->showline[lastfloor][2]->setText(QString::number(correct[lastfloor]));
    this->showline[lastfloor][3]->setText(QString::number(floorscore[lastfloor]));
    ui->TotalScoreLabel->setText("Total score : " + QString::number(totalscore));

    if(floorseed[lastfloor] != 1024){
        do{
            floorseed[lastfloor]=random.bounded(UINT_MAX);
            //自訂義碼, 學生不用修改
            if(floorseed[lastfloor] == 1024)
                continue;
            command.sprintf("select exists(select * from testdataresult where indexfloor = %d and seed = %u)", lastfloor, floorseed[lastfloor]);
            query.exec(command);
            query.next();
        }while((query.value(0).toInt() == 1));
    }

    command.sprintf("update floorinfrecover set \
                    totalcorrect = %d, \
                    totalque = %d, \
                    totalscore = %d, \
                    totalcosttime = %lld, \
                    currentseed = %u where indexfloor = %d",
                    correct[lastfloor],
                    quesnum[lastfloor],
                    floorscore[lastfloor],
                    costlist[lastfloor],
                    floorseed[lastfloor], lastfloor);
    query.exec(command);

    return correctans;
}

void JudgeWindow::scheduleEnd(){
    ui->TotalPathLabel->setText("Path length : "+QString::number(distance));
    QSqlQuery query(db);
    query.exec("use final");
    query.exec("delete from peoplelist where id regexp '^99999';");
    query.exec("drop table totalresultrecover");
    query.exec("drop table floorinfrecover");

    db.close();
}

JudgeWindow::~JudgeWindow()
{
    db.close();
    ofstream out("time.txt");

    for(int i=0;i<27;i++){
        out<<i<<","<<costlist[i]<<","<<correct[i]<<","<<quesnum[i]<<endl;
    }
    out.close();
    delete ui;
}

void JudgeWindow::on_pushButton_clicked()
{
}


/*
#include "judgewindow.h"
#include "ui_judgewindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <iostream>
#include <string>
#include <QGridLayout>
#include <QDebug>
#include <building.h>
#include <math.h>
#include <QString>
using namespace std;

JudgeWindow::JudgeWindow(QWidget *parent) :QWidget(parent),ui(new Ui::JudgeWindow)
{
    ui->setupUi(this);
    //read initial data
    query.exec("drop database FINAL");
    query.exec("create database if not exists FINAL");
    query.exec("use FINAL");
    query.exec("drop table if exists TestData");
    query.exec("create table if not exists TestData (ID char(8),Floor int,Question text,Answer text,PRIMARY KEY(ID))");
    query.exec("drop table if exists InitialCondition");
    query.exec("create table if not exists InitialCondition (ID char(8),Nowfloor int,Destination int,Number int,PRIMARY KEY(ID))");
    query.exec("load data infile 'C:/ProgramData/MySQL/MySQL Server 8.0/Uploads/testdata_1225.csv' into table TestData fields terminated by ',' enclosed by '\"' lines terminated by '\r\n' ignore 1 rows");
    query.exec("load data infile 'C:/ProgramData/MySQL/MySQL Server 8.0/Uploads/simple_initial_condition.csv' into table InitialCondition fields terminated by ',' enclosed by '\"' lines terminated by '\r\n' ignore 1 rows");
    //query.exec("load data infile 'C:/ProgramData/MySQL/MySQL Server 8.0/Uploads/initial_condition.csv' into table InitialCondition fields terminated by ',' enclosed by '\"' lines terminated by '\r\n' ignore 1 rows");
    query.exec("select * from InitialCondition");
    //set up initial array for disply result
    for(int i=0;i<27;i++){
        for(int j=0;j<4;j++){
            if(j==0){
                query.next();
                arr[i][j] = query.value(3).toInt();
                showline[i][j].setText(QString::number(arr[i][j]));
            }
            else if(j==1){
                arr[i][j] = 0;
                showline[i][j].setText(QString::number(0));
            }
            else if(j==2){
                arr[i][j] = 0;
            }
            else if(j==3){
                arr[i][j] = 0;
            }
            ui->gridLayout_2->addWidget(&showline[i][j],i,j);
        }
    }
    //=========================================想giveup哪一題===================================================
    for(int i=0;i<27;i++){
        for(int j=0;j<1;j++){

            if(i==22||i==23||i==26||i==25||i==4||i==20 ){ //check for giveup
                boxlist[i][j].setCheckState(Qt::Checked);
            }
            else boxlist[i][j].setCheckState(Qt::Unchecked);

            ui->gridLayout_checkbox->addWidget(&boxlist[i][j],i,j);
            floorLabel[i][j].setText(QString::number(i+1));
            ui->gridLayout_label->addWidget(&floorLabel[i][j],i,j);
        }
    }
    //========================================================================================================
    //========================================可在此測試某一題(除了該題外全部guveup)===========================
//    for(int i=0;i<27;i++){
//        for(int j=0;j<1;j++){
//            if(i==21){
//                boxlist[i][j].setCheckState(Qt::Unchecked);
//            }
//            else boxlist[i][j].setCheckState(Qt::Checked);

//            ui->gridLayout_checkbox->addWidget(&boxlist[i][j],i,j);
//            floorLabel[i][j].setText(QString::number(i+1));
//            ui->gridLayout_label->addWidget(&floorLabel[i][j],i,j);
//        }
//    }
    //===========================================================================================================

    //floor datatimes(每筆testdata測試10次)、floornextdata(每題執行到哪裡)
    for(int i=0;i<27;i++){
        floordatatimes[i] = 10;
        floornextdata[i] = 0;
        pass[i] = 0;
     }
    //set name label
    ui->name1->setText("陳舫慶");
    ui->id1->setText("0611069");
    ui->name2->setText("曾力揚");
    ui->id2->setText("0611097");
//    ui->name1->setText("嗨嗨嗨");
//    ui->id1->setText("1234568");
//    ui->name2->setText("嘿嘿嘿");
//    ui->id2->setText("1234567");
    for(int i=0;i<27;i++){
        for(int j=0;j<4;j++){
            cout<<arr[i][j]<<" ";
        }
        cout<<endl;
    }
    score = 0;
}

//取每層樓人的條件
void JudgeWindow::getInitialCondition(People *people[]){
    query.exec("use FINAL");
    //query.exec("select n.* from (select * from peoplelist limit "+QString::number(n-1)+",1) as s,peoplelist as n where left(n.id,5)=left(s.id,5)");
    query.exec("select * from InitialCondition");
    for(int i=0;i<27;i++){
        people[i] = new People;
        people[i]->setPeople(query);
    }
}

//取題目
string JudgeWindow::getData(int floor, int b,int& datatimes){
    //一筆測資跑幾遍 回傳給datatimes
    datatimes = floordatatimes[floor];
    //disable checkbox
    for(int i=0;i<27;i++){
        boxlist[i][0].setDisabled(true);
    }

    //看該floor的測資跑到第幾筆了
    int num = floornextdata[floor-1];
    cout<<floor<<": "<<num<<endl;
    floornextdata[floor-1]++;     //in order
    string result="";

    //give up 則output giveup
    if(boxlist[floor-1][0].isChecked()){
        result = "GIVEUP";
        answer="";
    }
    //要執行的取data(按照floornextdata的順序)
    else{
        query.exec("use FINAL");
        //query.exec("select count(Question) from testdata where floor="+QString::number(floor+1));
        //query.next();
        //cerr<<query.value(0).toInt()<<endl;
        //    int num = rand()%(query.value(0).toInt());
        query.exec("select Question,Answer from testdata where floor = "+QString::number(floor)+" and ID like '%0"+QString::number(num)+"'");
        query.next();
        result=query.value(0).toString().toStdString();
        answer=query.value(1).toString().toStdString();
    }
    //進電梯，該樓層waiting-1
    if(b==1){
        arr[floor-1][0]--;
    }
    //出電梯，該樓層arrive+1
    else if(b == 0){
        arr[floor-1][1]++;
    }
    //開始計時
    timer.start();
    return result;
}

bool JudgeWindow::submitData(int floor,string ans){
    //timer end，計算花費的時間並更新
    costtime = timer.nsecsElapsed();
    costtime /= floordatatimes[floor-1];
    arr[floor-1][2]+=costtime;

    //比較答案，計算分數，並更新judgewindoe介面(display)
    if(answer.compare(ans)==0){
        score = 10000000000+pow(2,floornextdata[floor-1]-1);
        cout<<"score(floor"<<floor<<": "<<score<<endl;
        arr[floor-1][3]+=score;
        pass[floor-1]++;
        display(floor);
        return true;
    }
    else{
        //錯了嗚嗚
        return false;
    }
}

int JudgeWindow::getConditionNum(){
    return rand()%300+1;
}

JudgeWindow::~JudgeWindow()
{
    delete ui;
}

//update judgewindow
void JudgeWindow::display(int nowfloor){
    for(int i=0;i<27;i++){
        for(int j=0;j<4;j++){
            if(j==0){   //waiting
                showline[i][j].setText(QString::number(arr[i][j]));
            }
            else if(j==1){   //arriving
                showline[i][j].setText(QString::number(arr[i][j]));
            }
            else if(j==2 && i==nowfloor-1){
                showline[i][j].setText(QString::number(arr[i][j]));

            }
            if(j==3 && i==nowfloor-1){
                showline[i][j].setText(QString::number(arr[i][j]));
            }
        }
    }
}

//export
void JudgeWindow::on_export_to_database_clicked()
{
    QSqlDatabase::removeDatabase("QMYSQL");
    database2 = QSqlDatabase::addDatabase("QMYSQL");
    database2.setHostName("140.113.146.120");
    database2.setUserName("aoopstudent");
    database2.setPassword("tsaimother");
    database2.setPort(3306);
    bool ok = database2.open();
    if(ok){
        qDebug()<<"Connected!!";
    }
    else{
        qDebug()<<"fail to connect";
    }

    QSqlQuery query(database2);
    QString name1=ui->name1->text(),name2=ui->name2->text(),id1=ui->id1->text(),id2=ui->id2->text();
    QString totalStep = QString::number(distance);
    //query.exec("select * from totalpath");
    query.exec("insert into aoopstudentuse.totalpath (stud_id1,stud_name1,stud_id2,stud_name2,pathlen) values ('"+id1+"','"+name1+"','"+id2+"','"+name2+"',"+totalStep+")");
    for(int i=0;i<27;i++){
        query.exec("insert into aoopstudentuse.floorscore(stud_id1,stud_name1,stud_id2,stud_name2,floor,timespent,pass,totalques,totalscore) values ('"+id1+"','"+name1+"','"+id2+"','"+name2+"',"+QString::number(i+1)+","+QString::number(arr[i][2])+","+QString::number(pass[i])+","+QString::number(floornextdata[i])+","+QString::number(arr[i][3])+")");
    }
}
*/
