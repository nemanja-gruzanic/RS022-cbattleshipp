#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtMath>
#include <QMessageBox>
#include <QString>
#include <QJsonArray>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QFontDatabase>

#define CELL_SIZE 30

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setUiFonts();

    setCellSize();

    // set up table items
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            ui->playerShips->setItem(i, j, new QTableWidgetItem);
            ui->opponentShips->setItem(i, j, new QTableWidgetItem);
        }
    }

    disablePlayerButtons();

    // connecting button's signals and corresponding slots
    connect(ui->playButton, SIGNAL(clicked(bool)), this, SLOT(onPlayClicked()));
    connect(ui->sendButton, SIGNAL(clicked(bool)), this, SLOT(onSendClicked()));

    connect(ui->boatSize2, SIGNAL(clicked(bool)), this, SLOT(setBoatSize2()));
    connect(ui->boatSize3, SIGNAL(clicked(bool)), this, SLOT(setBoatSize3()));
    connect(ui->boatSize4, SIGNAL(clicked(bool)), this, SLOT(setBoatSize4()));
    connect(ui->boatSize5, SIGNAL(clicked(bool)), this, SLOT(setBoatSize5()));

    connect(ui->playerShips, SIGNAL(cellClicked(int,int)), this, SLOT(onCellClick(int,int)));

    connect(ui->ReadyButton, SIGNAL(clicked(bool)), this, SLOT(onReadyToPlayButtonClicked()));
    connect(ui->hitButton, SIGNAL(clicked(bool)), this, SLOT(onHitButtonClicked()));
    connect(ui->opponentShips, SIGNAL(cellClicked(int, int)), this, SLOT(onOpponentCellClicked(int, int)));

    connect(ui->quitButton, SIGNAL(clicked(bool)), this, SLOT(onQuitClicked()));

    // set application background
    QPixmap background(":/images/background.png");

    background = background.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Background, background);
    this->setPalette(palette);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onPlayClicked()
{
    // set player name
    m_player.name(ui->lePlayerName->text());

    if (m_player.connectToHost(ui->leServerIP->text())) {
        connect(m_player.m_socket.get(), SIGNAL(readyRead()), this, SLOT(recieveServerMsg()));

        ui->displayManager->setCurrentWidget(ui->gameScreen);
        ui->laTablePlayerName->setText(m_player.name());

        ui->teNotifications->append("Connected");
        ui->teNotifications->append("Waiting for other player to join ...");

        // send play request
        QJsonObject request;
        request.insert("iw2p", 1);
        request.insert("name", m_player.name());

        QJsonDocument document(request);

        m_player.m_socket->write(document.toJson());
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Server could not be found!");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }
}


void MainWindow::onSendClicked()
{
    // send chat request
    if (!ui->leTextMsg->text().isEmpty()) {
        // print message to your chat box
        ui->teChat->append(QString("[" + m_player.name() + "]: " + ui->leTextMsg->text()));

        // send message to other player
        // message format [player_name]: msg_content
        QJsonObject request;
        request.insert("chat_msg", "[" + m_player.name() + "]: " + ui->leTextMsg->text());
        request.insert("player_type", m_player.m_playerType);
        request.insert("game_id", m_player.m_gameId);

        QJsonDocument doc(request);
        m_player.m_socket->write(doc.toJson());

        ui->leTextMsg->clear();
    }
}


void MainWindow::setBoatSize2()
{
    if(m_availableShipsSize2 > 0)
        m_boatSize=2;
    qDebug() << m_boatSize;
}


void MainWindow::setBoatSize3()
{
    if(m_availableShipsSize3 > 0)
        m_boatSize=3;
    qDebug() << m_boatSize;
}


void MainWindow::setBoatSize4()
{
    if(m_availableShipsSize4 > 0)
        m_boatSize=4;
    qDebug() << m_boatSize;
}


void MainWindow::setBoatSize5()
{
    if(m_availableShipsSize5 > 0)
        m_boatSize=5;
    qDebug() << m_boatSize;
}


void MainWindow::onCellClick(int y, int x)
{
    //user must first click on ship size before placing it
    if(m_boatSize!=0){
        bool cellUsed = false;
        //second click
        if(m_selectedCell){
            //set boat
            if((qFabs(x - m_x1) == 1 && y - m_y1 == 0) || (qFabs(y - m_y1) == 1 && x - m_x1 == 0)){
                //right
                if(x > m_x1 && m_x1 + m_boatSize - 1 <= 9){
                    for(int i = 0; i < m_boatSize; i++){
                        cellUsed = cellUsed || ui->playerShips->item(m_y1, m_x1 + i)->isSelected();
                    }
                    if(!cellUsed){
                        for(int i = 0; i < m_boatSize; i++){
                            ui->playerShips->item(m_y1, m_x1 + i)->setSelected(true);
                        }
                        reduceBoatCount();
                    }
                }
                //left
                if(x < m_x1 && m_x1 - m_boatSize + 1 >= 0){
                    for(int i = 0; i < m_boatSize; i++){
                        cellUsed = cellUsed || ui->playerShips->item(m_y1, m_x1 - i)->isSelected();
                    }
                    if(!cellUsed){
                        for(int i = 0; i < m_boatSize; i++){
                            ui->playerShips->item(m_y1, m_x1 - i)->setSelected(true);
                        }
                        reduceBoatCount();
                    }
                }
                //down
                if(y > m_y1 && m_y1 + m_boatSize - 1 <= 9){
                    for(int i = 0; i < m_boatSize; i++){
                        cellUsed = cellUsed || ui->playerShips->item(m_y1 + i, m_x1)->isSelected();
                    }
                    if(!cellUsed){
                        for(int i = 0; i < m_boatSize; i++){
                            ui->playerShips->item(m_y1 + i, m_x1)->setSelected(true);
                        }
                        reduceBoatCount();
                    }
                }
                //up
                if(y < m_y1 && m_y1 - m_boatSize + 1 >= 0){
                    for(int i = 0; i < m_boatSize; i++){
                        cellUsed = cellUsed || ui->playerShips->item(m_y1 - i, m_x1)->isSelected();
                    }
                    if(!cellUsed){
                        for(int i = 0; i < m_boatSize; i++){
                            ui->playerShips->item(m_y1 - i, m_x1)->setSelected(true);
                        }
                        reduceBoatCount();
                    }
                }

                // reset marker variable
                deleteGray(m_y1,m_x1);
                m_selectedCell = false;
                m_boatSize = 0;
            }
            else {
                deleteGray(m_y1,m_x1);
                m_x1 = -2;
                m_y1 = -2;
                m_selectedCell = false;
            }

        }
        //first click
        else if(!ui->playerShips->item(y, x)->isSelected()){
            if(x != 0 && x - m_boatSize + 1 >= 0 && leftGrayCell(m_boatSize, y, x)){
                ui->playerShips->item(y, x - 1)->setBackground(Qt::gray);
            }
            if(x != 9 && x + m_boatSize - 1 <= 9 && rightGrayCell(m_boatSize, y, x)){
                ui->playerShips->item(y, x + 1)->setBackground(Qt::gray);
            }
            if(y != 0 && y - m_boatSize + 1 >= 0 && upGrayCell(m_boatSize, y, x)){
                ui->playerShips->item(y - 1, x)->setBackground(Qt::gray);
            }
            if(y != 9 && y + m_boatSize - 1 <= 9 && downGrayCell(m_boatSize, y, x)){
                ui->playerShips->item(y + 1, x)->setBackground(Qt::gray);
            }
            m_selectedCell = true;
            m_x1 = x;
            m_y1 = y;
        }
    }
}

void MainWindow::reduceBoatCount()
{
    if(m_boatSize == 2){
        m_availableShipsSize2--;
        ui->countSize2->setText(QString::number(m_availableShipsSize2));
        qDebug() << m_availableShipsSize2 << "brodova2";
    }
    if(m_boatSize == 3){
        m_availableShipsSize3--;
        ui->countSize3->setText(QString::number(m_availableShipsSize3));
        qDebug() << m_availableShipsSize3 << "brodova3";
    }
    if(m_boatSize == 4){
        m_availableShipsSize4--;
        ui->countSize4->setText(QString::number(m_availableShipsSize4));
        qDebug() << m_availableShipsSize4 << "brodova4";
    }
    if(m_boatSize == 5){
        m_availableShipsSize5--;
        ui->countSize5->setText(QString::number(m_availableShipsSize5));
        qDebug() << m_availableShipsSize5 << "brodova5";
    }
}

void MainWindow::resetGameUi()
{
    disablePlayerButtons();


    ui->teChat->clear();

    // reset notifications
    ui->teNotifications->clear();
    ui->teNotifications->append("Wait for other player to joint the game.");

    // reset player's ships
    for (int i = 0; i < ui->playerShips->rowCount(); ++i) {
        for (int j = 0; j < ui->playerShips->columnCount(); ++j) {
            if (ui->playerShips->item(i, j)->isSelected())
                ui->playerShips->item(i, j)->setSelected(false);
        }
    }

    for (int i = 0; i < ui->playerShips->rowCount(); ++i) {
        for (int j = 0; j < ui->playerShips->columnCount(); ++j) {
            ui->playerShips->item(i, j)->setBackgroundColor(Qt::white);
        }
    }

    // reset ship counters
    m_availableShipsSize2 = 4;
    m_availableShipsSize3 = 3;
    m_availableShipsSize4 = 2;
    m_availableShipsSize5 = 1;

    ui->countSize2->setText(QString::number(4));
    ui->countSize3->setText(QString::number(3));
    ui->countSize4->setText(QString::number(2));
    ui->countSize5->setText(QString::number(1));

    m_player.m_shipsLeft = 30;
    ui->laPlayerShipsLeft->setText(QString::number(30));

    // reset opponent's ships
    ui->laTableOpponentName->setText("Opponent name");

    m_player.m_opponentShipsLeft = 30;
    ui->laOpponentShipsLeft->setText(QString::number(30));

    for (int i = 0; i < ui->opponentShips->rowCount(); ++i) {
        for (int j = 0; j < ui->opponentShips->columnCount(); ++j) {
            ui->opponentShips->item(i, j)->setBackgroundColor(Qt::white);
        }
    }
}

void MainWindow::deleteGray(int y, int x){
    //right
    if(x != 9)
        ui->playerShips->item(y, x + 1)->setBackground(Qt::white);
    //left
    if(x != 0)
        ui->playerShips->item(y, x - 1)->setBackground(Qt::white);
    //down
    if(y != 9)
        ui->playerShips->item(y + 1, x)->setBackground(Qt::white);
    //up
    if(y != 0)
        ui->playerShips->item(y - 1, x)->setBackground(Qt::white);
}


void MainWindow::recieveServerMsg()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());

    QJsonDocument msg = QJsonDocument::fromJson(socket->readAll());
    QJsonObject response = msg.object();

    qDebug() << response;

    if (response.isEmpty())
        return;

    // server will send one response at a time = return statements
    if (response.contains("ucp")) {
        handlePlayResponse(response);
        return;
    }

    if (response.contains("chat_msg")) {
        handleChatResponse(response);
        return;
    }

    if (response.contains("od")) {
        handleOpponentDisconnectedResponse(response);
        return;
    }

    if (response.contains("wait_opp")) {
        handleWaitOpponentResponse(response);
        return;
    }

    if (response.contains("opp_ready")) {
        handleReadyOpponentResponse(response);
        return;
    }

    if (response.contains("start_game")) {
        handleGameStartResponse(response);
        return;
    }

    if (response.contains("attack"))
        handleAttackResponse(response);

    if (response.contains("if_hit"))
        handleIfHitResponse(response);
}

bool MainWindow::leftGrayCell(int size, int y, int x)
{
    bool gray = true;
    for(int i = 0; i < size; i++){
        if(ui->playerShips->item(y, x - i)->isSelected())
            gray = false;
    }
    return gray;
}

bool MainWindow::rightGrayCell(int size, int y, int x)
{
    bool gray = true;
    for(int i = 0; i < size; i++){
        if(ui->playerShips->item(y, x + i)->isSelected())
            gray = false;
    }
    return gray;
}

bool MainWindow::downGrayCell(int size, int y, int x)
{
    bool gray = true;
    for(int i = 0; i < size; i++){
        if(ui->playerShips->item(y + i, x)->isSelected())
            gray = false;
    }
    return gray;
}

bool MainWindow::upGrayCell(int size, int y, int x)
{
    bool gray = true;
    for(int i = 0; i < size; i++){
        if(ui->playerShips->item(y - i, x)->isSelected())
            gray = false;
    }
    return gray;
}

void MainWindow::handleQuit()
{
    QMessageBox msgBox;
    msgBox.setText("Are you sure?");
    msgBox.setIcon(QMessageBox::Warning);

    QAbstractButton* buttonYes = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
    QAbstractButton* buttonNo = msgBox.addButton(tr("No"), QMessageBox::NoRole);

    msgBox.exec();

    if (msgBox.clickedButton() == buttonYes) {
        QApplication::quit();
    }

    if (msgBox.clickedButton() == buttonNo) {
        return;
    }
}

void MainWindow::handlePlayResponse(QJsonObject & response)
{
    // set player type and game id
    m_player.m_playerType = response.value("player_type").toInt();
    m_player.m_gameId = response.value("game_id").toInt();

    // inform that other player has joined and that game has started
    ui->teNotifications->append("Player " + response.value("opp_name").toString() + " joined!");
    ui->laTableOpponentName->setText(response.value("opp_name").toString());

    ui->teNotifications->append("Place your boats and then click on Ready.");

    // enable disabled buttons
    ui->playerShips->setEnabled(true);
    ui->ReadyButton->setEnabled(true);
    ui->boatSize2->setEnabled(true);
    ui->boatSize3->setEnabled(true);
    ui->boatSize4->setEnabled(true);
    ui->boatSize5->setEnabled(true);
}

void MainWindow::handleChatResponse(QJsonObject & response)
{
    ui->teChat->append(response.value("chat_msg").toString());
}

void MainWindow::handleOpponentDisconnectedResponse(QJsonObject & response)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("Opponent has left the game. Play again?");

    QAbstractButton* buttonYes = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
    QAbstractButton* buttonNo = msgBox.addButton(tr("Quit"), QMessageBox::NoRole);

    msgBox.exec();

    QJsonObject msg;

    if (msgBox.clickedButton() == buttonYes) {
        resetGameUi();

        msg.insert("play_again", 1);
        msg.insert("player_type", m_player.m_playerType);
        msg.insert("game_id", m_player.m_gameId);

        msg.insert("game_outcome", GameOutcome::FAILURE);

        QJsonDocument doc(msg);
        m_player.m_socket->write(doc.toJson());
    }

    if (msgBox.clickedButton() == buttonNo) {
        QApplication::quit();
    }
}

void MainWindow::handleWaitOpponentResponse(QJsonObject &response)
{
    ui->teNotifications->append("Wait for " + response.value("opp_name").toString() + " to get ready.");
}

void MainWindow::handleReadyOpponentResponse(QJsonObject &response)
{
    ui->teNotifications->append(response.value("opp_name").toString() + " is ready.");
}

void MainWindow::handleGameStartResponse(QJsonObject &response)
{
   ui->teNotifications->append("Game starts!");
   ui->teNotifications->append(response.value("turn").toString() + " turn.");

   // set player turn
   if (!response.value("turn").toString().compare(m_player.m_name)) {
       m_turn = true;
       ui->opponentShips->setEnabled(true);
       ui->hitButton->setEnabled(true);
   }
   else {
       m_turn = false;
       ui->hitButton->setEnabled(false);
   }
}

void MainWindow::setUiFonts()
{
    // include fonts
    QFontDatabase::addApplicationFont(":/fonts/ArcadeRounded.ttf");

    QFont font("Arcade Rounded", 12, QFont::Normal);

    /* *  welcome screen * */

    // labels
    ui->laServerIP->setFont(font);
    ui->laPlayerName->setFont(font);

    // play button
    font.setPixelSize(24);
    ui->playButton->setFont(font);

    // game title
    font.setPixelSize(70);
    ui->laGameTitle->setFont(font);

    /* * game screen * */

    // player names
    font.setPixelSize(12);
    ui->laTablePlayerName->setFont(font);
    ui->laTableOpponentName->setFont(font);

    // available ships counters and corresponding buttons
    font.setPixelSize(9);
    ui->laShipsLeft->setFont(font);

    font.setPixelSize(12);
    ui->countSize2->setFont(font);
    ui->countSize3->setFont(font);
    ui->countSize4->setFont(font);
    ui->countSize5->setFont(font);

    ui->boatSize2->setFont(font);
    ui->boatSize3->setFont(font);
    ui->boatSize4->setFont(font);
    ui->boatSize5->setFont(font);

    // ship counters
    ui->laPlayersShips->setFont(font);
    ui->laPlayerShipsLeft->setFont(font);

    ui->laOpponentShips->setFont(font);
    ui->laOpponentShipsLeft->setFont(font);

    // buttons
    ui->hitButton->setFont(font);
    ui->ReadyButton->setFont(font);
    ui->quitButton->setFont(font);
    ui->sendButton->setFont(font);

    // chat and notification labels
    ui->laChat->setFont(font);
    ui->laNotifications->setFont(font);

    // tables
    font.setPixelSize(8);
    ui->playerShips->setFont(font);
    ui->opponentShips->setFont(font);
}

void MainWindow::setCellSize()
{
    // set player's cells width and height
    ui->playerShips->horizontalHeader()->setMinimumSectionSize(CELL_SIZE);
    ui->playerShips->verticalHeader()->setMinimumSectionSize(CELL_SIZE);
    ui->playerShips->horizontalHeader()->setDefaultSectionSize(CELL_SIZE);
    ui->playerShips->verticalHeader()->setDefaultSectionSize(CELL_SIZE);

    // set opponent's cells width and height
    ui->opponentShips->horizontalHeader()->setMinimumSectionSize(CELL_SIZE);
    ui->opponentShips->verticalHeader()->setMinimumSectionSize(CELL_SIZE);
    ui->opponentShips->horizontalHeader()->setDefaultSectionSize(CELL_SIZE);
    ui->opponentShips->verticalHeader()->setDefaultSectionSize(CELL_SIZE);
}


void MainWindow::disablePlayerButtons()
{
    // disable game buttons before game starts
    ui->playerShips->setDisabled(true);
    ui->opponentShips->setDisabled(true);
    ui->hitButton->setDisabled(true);
    ui->ReadyButton->setDisabled(true);
    ui->boatSize2->setDisabled(true);
    ui->boatSize3->setDisabled(true);
    ui->boatSize4->setDisabled(true);
    ui->boatSize5->setDisabled(true);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    handleQuit();
}


void MainWindow::onReadyToPlayButtonClicked()
{
    //Check if all ships are set up
    if(m_availableShipsSize2 != 0 || m_availableShipsSize3 !=0 || m_availableShipsSize4 != 0 || m_availableShipsSize5 != 0 ){

        QMessageBox msgBox;
        msgBox.setText("Ships are not set up!");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();

    }else{
        //Send to server map of ships
        QJsonObject mapOfShips;

        mapOfShips.insert("ready", 1);
        mapOfShips.insert("gameId", m_player.m_gameId);
        mapOfShips.insert("playerType", m_player.m_playerType);

        for(int i=0 ; i<ui->playerShips->rowCount(); ++i){

            QJsonArray niz;

            for (int j = 0; j < ui->playerShips->columnCount(); ++j) {

                if(ui->playerShips->item(i,j)->isSelected()){
                    niz.append(1);
                }else{
                    niz.append(0);
                }

            }
            mapOfShips.insert(QString::number(i), niz);
        }

        QJsonDocument doc(mapOfShips);
        m_player.m_socket->write(doc.toJson());

        qDebug() << doc;

        // disable ReadyButton so player cannot send multiple ready requests
        ui->ReadyButton->setDisabled(true);
    }
}

void MainWindow::onOpponentCellClicked(int y, int x)
{
    m_opponentSelectedCell = true;

    // check if player selected multiple cells
    for(int i = 0; i < ui->opponentShips->rowCount(); ++i) {
        for (int j = 0; j < ui->opponentShips->columnCount(); ++j) {
            if (ui->opponentShips->item(i,j)->isSelected())
                ui->opponentShips->item(i, j)->setSelected(false);
        }
    }

    m_ox = x;
    m_oy = y;


    // select field that player want to hit
    ui->opponentShips->item(m_oy, m_ox)->setSelected(true);
}

void MainWindow::onQuitClicked()
{
    handleQuit();
}

void MainWindow::onHitButtonClicked()
{
    ui->opponentShips->item(m_oy, m_ox)->setSelected(false);

    if(m_opponentSelectedCell){

        // if player selected already hitted spot
        if (ui->opponentShips->item(m_oy, m_ox)->backgroundColor() == Qt::gray ||
            ui->opponentShips->item(m_oy, m_ox)->backgroundColor() == Qt::red ) {
            QMessageBox msgBox;
            msgBox.setText("You can't hit the same spot twice!");
            msgBox.setIcon(QMessageBox::Icon::Critical);

            msgBox.exec();

            // reset old opponent ship coordinates
            m_ox = -1;
            m_oy = -1;
        }
        // send hit request
        else {
            QJsonObject playerHit;
            playerHit.insert("hit", 1);
            playerHit.insert("x", m_ox);
            playerHit.insert("y", m_oy);

            playerHit.insert("gameId", m_player.m_gameId);
            playerHit.insert("playerType", m_player.m_playerType);

            m_turn = false;
            ui->hitButton->setEnabled(false);

            QJsonDocument doc(playerHit);
            m_player.m_socket->write(doc.toJson());

            m_oldX = m_ox;
            m_oldY = m_oy;
        }
    }else{

        QMessageBox msgBox;
        msgBox.setText("Choose a cell!");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }

    m_opponentSelectedCell = false;
}

void MainWindow::handleAttackResponse(QJsonObject & response)
{
    // read hit info
    m_oppHitX = response.value("x").toInt();
    m_oppHitY = response.value("y").toInt();

    if(response.contains("yah")){

        ui->teNotifications->append("You are hit!");
        m_player.m_shipsLeft--;
        ui->laPlayerShipsLeft->setText(QString::number(m_player.m_shipsLeft));

        // unselect ship, and color it
        ui->playerShips->item(m_oppHitY, m_oppHitX)->setSelected(false);
        ui->playerShips->item(m_oppHitY, m_oppHitX)->setBackgroundColor(Qt::red);

        if(m_player.m_shipsLeft == 0){
            QMessageBox::StandardButton replay = QMessageBox::question(this,"Game over" ,"You lost the game! Rematch?",
                                                                                   QMessageBox::Yes | QMessageBox::No);
            if(replay == QMessageBox::Yes){
                resetGameUi();

                QJsonObject playAgain;
                playAgain.insert("play_again", 1);
                playAgain.insert("player_type", m_player.m_playerType);
                playAgain.insert("game_id", m_player.m_gameId);

                playAgain.insert("game_outcome", GameOutcome::SUCCESSFUL);
                QJsonDocument doc(playAgain);
                m_player.m_socket->write(doc.toJson());

            }else{
                QApplication::quit();
            }

        }

    }else{

        ui->teNotifications->append("You are not hit!");
        m_turn = true;
        ui->hitButton->setEnabled(true);
        ui->opponentShips->setEnabled(true);

        ui->playerShips->item(m_oppHitY, m_oppHitX)->setBackgroundColor(Qt::gray);
    }
}

void MainWindow::handleIfHitResponse(QJsonObject & response)
{
    if(response.contains("great_attack")){

        ui->hitButton->setEnabled(true);
        m_turn = true;
        ui->teNotifications->append("You hit your opponent!");
        ui->opponentShips->item(m_oy, m_ox)->setBackgroundColor(Qt::red);
        m_player.m_greatAttack++;
        m_player.m_opponentShipsLeft--;
        ui->laOpponentShipsLeft->setText(QString::number(m_player.m_opponentShipsLeft));

        if(m_player.m_greatAttack == 30){
            QMessageBox::StandardButton replay = QMessageBox::question(this,"Game over" ,"VICTORY! Rematch?",
                                                                                   QMessageBox::Yes | QMessageBox::No);
            if(replay == QMessageBox::Yes){
                resetGameUi();

                QJsonObject playAgain;
                playAgain.insert("play_again", 1);
                playAgain.insert("player_type", m_player.m_playerType);
                playAgain.insert("game_id", m_player.m_gameId);

                playAgain.insert("game_outcome", GameOutcome::SUCCESSFUL);
                QJsonDocument doc(playAgain);
                m_player.m_socket->write(doc.toJson());

            }else{
                QApplication::quit();
            }

        }
    }else{
        ui->teNotifications->append("You did't hit your opponent!");
        ui->opponentShips->setEnabled(false);
        ui->opponentShips->item(m_oy, m_ox)->setBackgroundColor(Qt::gray);
    }
}
