
#include <iostream>
#include <string>
#include <bitset>
#include <math.h>
#include "kademlia.hpp"

#include <stdio.h>
#include <sqlite3.h>


Kademlia::Kademlia(std::string node_name_input, std::string data_input, std::string kademliaID, sqlite3* inputDB)
{
    node_name = node_name_input;
    dataList = new Data();
    dataList->head = dataList;
    dataList->Name = data_input;
    queryList = new Data();
    queryList->head = queryList;
    queryList->Name = "init";
    KId = kademliaID;
    db = inputDB;
    dataList->db = inputDB;
    queryList->db = inputDB;
    
}

bool
Kademlia::GetData(std::string DataName){
    Data *output = NULL;
    output = dataList->GetData(DataName, GetKId());
    if (output == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

std::string
Kademlia::GetNext_Node(std::string TargetNode, int output_num, std::string SourceNode){

    std::string output[3] = {this->KId, "NULL", "NULL"};
    std::string outputString = "";
    int arrIndex = 0;

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (SourceNode == k_bucket[i])
        {
            continue;
        }
        
        if (k_bucket[i] != "NULL")
        {
            if (output[arrIndex%3] == "NULL")
            {
                output[arrIndex%3] = k_bucket[i];
                arrIndex++;
            }
            else if (this->XOR(TargetNode, k_bucket[i]) < this->XOR(TargetNode, output[arrIndex%3]))
            {
                output[arrIndex%3] = k_bucket[i];
                arrIndex++;
            }
        } 
    }

    if (output_num == 1)
    {
        return output[(arrIndex-1)%3];
    }

    for (int i = 0; i < 3; i++)
    {
        if (output[i] != "NULL")
        {
            outputString = outputString + "_" + output[i];
        }

    }

    outputString = outputString + "_";
    
    //未完成，需要return output_num數量的Knode資訊
    return outputString;
}

int
Kademlia::XOR(std::string input)
{
    std::cout << "Xor: " << this->KId << " and " << input << "\n";

    int distance = 0;
    for (int i = 0; i < 8; i++)
    {
        std::string str1 = std::to_string(this->KId[i]);
        std::string str2 = std::to_string(input[i]);
        if (str1.compare(str2) == 0)
        {
            distance = distance + pow(2, 8-i);
        }
        
    }
    return distance;
}

int
Kademlia::XOR(std::string input, std::string input2)
{
    //std::cout << "Xor2: " << input2 << " and " << input << "\n";

    int distance = 0;
    for (int i = 0; i < 8; i++)
    {
        std::string str1 = std::to_string(input2[i]);
        std::string str2 = std::to_string(input[i]);
        if (str1.compare(str2) == 0)
        {
            distance = distance + pow(2, 8-i);
        }
        
    }
    return distance;
}

void
Kademlia::Node_info(){
    //std::cout << "**************" << std::endl;
    // std::cout << "node name = " << node_name << std::endl;
    // std::cout << "data = " << data << std::endl;
    // std::cout << "next node = " << next_node << std::endl;
    dataList->printAllData();
    //std::cout << std::endl << "**************" << std::endl;
}

void
Kademlia::Set_KBucket(std::string KNode)
{
    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (k_bucket[i] == "NULL")
        {
            k_bucket[i] = KNode;
            return;
        }
    }
}

void
Kademlia::SetData(std::string input, std::string type)
{
    dataList->AddData(input, type);
}

//以輸入的節點名稱比較其他K桶中資訊，並決定是否將其加入
std::string
Kademlia::KBucket_update(std::string sourceNode)
{
    if (sourceNode == KId)
    {
        return "NULL";
    }
    

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (k_bucket[i] == sourceNode)
        {
            //std::cout << "already have same node in K-buk\n";
            return "NULL";
        }    
    }

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (k_bucket[i] == "NULL")
        {
            k_bucket[i] = sourceNode;
            return "add sourceNode to a NULL";
        }
    }
    
    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        int distance = XOR(sourceNode, this->GetKId());
        if (XOR(k_bucket[i], this->GetKId()) < distance)
        {
            return k_bucket[i];
        }
        
    }

    //演算法需要決策出一個替換的節點
    return "K-buk is full";
}

//將輸入節點從k桶中去除
void
Kademlia::KBucket_delete(std::string sourceNode)
{
    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (k_bucket[i] == sourceNode)
        {
            k_bucket[i] = "NULL";
            return;
        }
        
    }
    
}

//從datalist中刪除資料
//需要修改
void
Kademlia::Delete_data(std::string DataName)
{
    Data* targetPtr = dataList->GetData(DataName, GetKId());

    if (targetPtr == NULL)
    {
        std::cout << "In delete_data(), GetData() is NULL\n";
        return;
    }

    // sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand; 

    // rc = sqlite3_open("test.db", &db);

    // if( rc ){
    //     fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    //     exit(0);
    // }
    
    sqlCommand = std::string("DELETE * from RECORD WHERE NODE=") + "'" + GetKId() + "'" + " AND DATA='" + DataName + "';";  
                 
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], dataList->DB_DeleteData, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
//    sqlite3_close(db);


    
    // Data* prePtr = dataList;

    // while (1)
    // {
    //     if (prePtr->next == targetPtr)
    //     {
    //         prePtr->next = targetPtr->next;
    //         delete targetPtr;
    //         return;
    //     }
    //     prePtr = prePtr->next;

    //     if (prePtr == NULL)
    //     {
    //        return;
    //     }
        
    // }
    
}

//從datalist中刪除資料
void
Kademlia::Delete_data_query(std::string DataName)
{
    Data* targetPtr = queryList->GetData(DataName);

    if (targetPtr == NULL)
    {
        std::cout << "In delete_data(), GetData() is NULL\n";
        return;
    }
    
    Data* prePtr = queryList;

    while (1)
    {
        if (prePtr->next == targetPtr)
        {
            std::cout << "Delete Data sucess: " << targetPtr->Name << "\n";
            prePtr->next = targetPtr->next;
            delete targetPtr;
            return;
        }
        prePtr = prePtr->next;

        if (prePtr == NULL)
        {
           return;
        }
        
    }
    
}

//針對輸入節點，比較所有更接近的資料，回傳整理的字串
//需要修改
std::string
Kademlia::Transform_Data(std::string thisNode, std::string targetNode)
{




    std::string output = "|";
    Data* DataPtr = dataList->next;

    while (DataPtr != NULL)
    {
        std::size_t hashData = std::hash<std::string>{}(DataPtr->Name);
        std::string binaryData = std::bitset<8>(hashData).to_string();
        int thisNode_distance = XOR(thisNode, binaryData);
        int targetNode_distance = XOR(targetNode, binaryData);

        if (targetNode_distance < thisNode_distance)
        {
            output = output + DataPtr->Name + "|" ;
        }
        DataPtr = DataPtr->next;
    }
    return output;
}



//---------------------Class Data-------------------------
//需要修改
void
Data::AddData(std::string inputName, std::string k_ID)
{
    // sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand; 

    // rc = sqlite3_open("test.db", &db);

    // if( rc ){
    //     fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    //     exit(0);
    // }
    
    sqlCommand = std::string("INSERT INTO RECORD (NODE,DATA)") +
                 "VALUES('"+ k_ID + "', '" + inputName + "');";
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], this->DB_addDATA, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
//    sqlite3_close(db);

    // Data *inputData = new Data();
    // Data *nextPtr = this->next;
    // inputData->next = nextPtr;
    // this->next = inputData;
    // inputData->head = this;
    // inputData->Name = inputName;
    // inputData->type = inputType;
}

void
Data::AddData(std::string inputName, std::string inputType, int Empty)
{

    Data *inputData = new Data();
    Data *nextPtr = this->next;
    inputData->next = nextPtr;
    this->next = inputData;
    inputData->head = this;
    inputData->Name = inputName;
    inputData->type = inputType;
}

void
Data::printAllData()
{
    std::cout << Name << "||";
    Data *tempPtr = this->next;
    while (tempPtr != NULL)
    {
        std::cout << tempPtr->Name << "||";
        tempPtr = tempPtr->next;
    }
    
}

Data*
Data::GetTail()
{
    Data *outputPtr = this;
    Data *tempPtr = this->next;
    while (tempPtr != NULL)
    {
        outputPtr = tempPtr;
        tempPtr = tempPtr->next;
    }
    return outputPtr;
}

Data*
Data::GetData(std::string DataName)
{
    Data *tempptr = this;
    while (tempptr != NULL)
    {
        if (DataName.compare(tempptr->Name) == 0)
        {
            return tempptr;
        }
        else
        {
            tempptr = tempptr->next;
        }
    }
    return NULL;
}

//從資料庫裡存取資料
Data*
Data::GetData(std::string DataName, std::string k_ID)
{
    //需要修改

    // sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand, sqlOutput; 
    int command_output = 0;

    // rc = sqlite3_open("test.db", &db);

    // if( rc ){
    //     fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    //     exit(0);
    // }
    
    sqlCommand = std::string("SELECT * from RECORD WHERE NODE=") + "'" + k_ID + "'" + " AND DATA='" + DataName + "';";  
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], this->DB_getDATA, &command_output, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
//    sqlite3_close(db);
    // std::cout << "command output = " <<command_output << "\n";
   if (command_output)
   {
       return this;
   }
   else
   {
       return NULL;
   }
   
   
}

void
Data::update_nextHop(std::string inputNode)
{
    int distance;
    std::size_t hashData = std::hash<std::string>{}(this->Name);
    std::string binaryData = std::bitset<8>(hashData).to_string();
    distance = XOR(binaryData,inputNode);

    if (distance >= XOR(binaryData, closest_node))
    {
        return;
    }

    for (int  i = 0; i < 3; i++)
    {
        if (this->nextHop_list[i] == "NULL")
        {
            this->nextHop_list[i] = inputNode;
            return;
        }
    }
    
    for (int i = 0; i < 3; i++)
    {
        if (this->nextHop_list[i] == inputNode)
        {
            // 預防nextHop list出現相同節點，重複Query
            return;
        }
        if (distance < XOR(binaryData, this->nextHop_list[i]))
        {
            this->nextHop_list[i] = inputNode;
            return;
        }     
    }
    
}

int
Data::XOR(std::string input, std::string input2)
{
    std::cout << "Xor data: " << input2 << " and " << input << "\n";
    int distance = 0;
    for (int i = 0; i < 8; i++)
    {
        std::string str1 = std::to_string(input2[i]);
        std::string str2 = std::to_string(input[i]);
        if (str1.compare(str2) == 0)
        {
            distance = distance + pow(2, 8-i);
        }
        
    }
    return distance;
}

//---------------------Class Order-------------------------

void Order::setNext(Order* ptr){
    nextPtr = ptr;
}


Order* Order::getTail(){
    Order *outputPtr = this;
    Order *tempPtr = this->getNext();
    while (tempPtr != NULL)
    {
        outputPtr = tempPtr;
        tempPtr = tempPtr->getNext();
    }
    return outputPtr;
}

void
Order::AddOrder(std::string orderName, std::string dataString, double timeString, int targetNumber)
{
    Order* inputData = new Order(orderName, dataString, timeString, targetNumber);
    Order* tempPtr = this;
    Order* prePtr = this;

    while (timeString > tempPtr->getTimeStamp() && tempPtr->getNext() != NULL)
    {
        prePtr = tempPtr;
        tempPtr = tempPtr->getNext();
    }
    if (tempPtr->getNext() == NULL && timeString > tempPtr->getTimeStamp())
    {
        tempPtr->setNext(inputData);
    }
    else
    {
        inputData->setNext(tempPtr);
        prePtr->setNext(inputData);
    }
}

Order*
Order::AddOrder_toTail(std::string orderName, std::string dataString, double timeString, int targetNumber)
{
    Order* inputData = new Order(orderName, dataString, timeString, targetNumber);
    Order* tempPtr = this;
    while (tempPtr->getNext() != NULL)
    {
        tempPtr = tempPtr->getNext();
    }
    tempPtr->setNext(inputData);
    return inputData;
}

//搜尋此order有無資料，有則回傳true，並修改dataList，fulfillList
bool
Order::checkDataList(std::string DataName)
{
    std::set<std::string> tempList = this->getDataList();
    if (tempList.find(DataName) != tempList.end())
    {
        tempList.erase(DataName);
        this->setFulfill_List(DataName);
        this->replace_Datalist(tempList);
        return true;
    }
    return false;
    
}
    
//確認此筆order是否完成
bool  
Order::checkFulFill()
{
    if (static_cast<int>(this->getDataList().size()) == 0)
    {
    //    std::cout << "******************" << std::endl;
    //    std::cout << "This Order is done!!!" << std::endl;
    //    std::cout << "******************" << std::endl;
       return true;
    }
    else
    {
        return false;
    }
    
}

void
Order::deleteOrder(Order* preOrder)
{
    preOrder->setNext(nextPtr);
    delete this;
}

