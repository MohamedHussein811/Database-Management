/*******************************************************************************
/* ODBCSQL: a sample program that implements an ODBC command line interpreter.
/*
/* USAGE:   ODBCSQL DSN=<dsn name>   or
/*          ODBCSQL FILEDSN=<file dsn> or
/*          ODBCSQL DRIVER={driver name}
/*
/*
/* Copyright(c)  Microsoft Corporation.   This is a WDAC sample program and
/* is not suitable for use in production environments.
/*
/******************************************************************************/
/* Modules:
/*      Main                Main driver loop, executes queries.
/*      DisplayResults      Display the results of the query if any
/*      AllocateBindings    Bind column data
/*      DisplayTitles       Print column titles
/*      SetConsole          Set console display mode
/*      HandleError         Show ODBC error messages
/******************************************************************************/

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <stdlib.h>
#include <sal.h>
#include <iostream>
#include <string>
using namespace std;

/*******************************************/
/* Macro to call ODBC functions and        */
/* report an error on failure.             */
/* Takes handle, handle type, and stmt     */
/*******************************************/

#define TRYODBC(h, ht, x)   {   RETCODE rc = x;\
                                if (rc != SQL_SUCCESS) \
                                { \
                                    HandleDiagnosticRecord (h, ht, rc); \
                                } \
                                if (rc == SQL_ERROR) \
                                { \
                                    fwprintf(stderr, L"Error in " L#x L"\n"); \
                                    goto Exit;  \
                                }  \
                            }
/******************************************/
/* Structure to store information about   */
/* a column.
/******************************************/

typedef struct STR_BINDING {
    SQLSMALLINT         cDisplaySize;           /* size to display  */
    WCHAR* wszBuffer;             /* display buffer   */
    SQLLEN              indPtr;                 /* size or null     */
    BOOL                fChar;                  /* character col?   */
    struct STR_BINDING* sNext;                 /* linked list      */
} BINDING;



/******************************************/
/* Forward references                     */
/******************************************/

void HandleDiagnosticRecord(SQLHANDLE      hHandle,
    SQLSMALLINT    hType,
    RETCODE        RetCode);

void DisplayResults(HSTMT       hStmt,
    SQLSMALLINT cCols);

void AllocateBindings(HSTMT         hStmt,
    SQLSMALLINT   cCols,
    BINDING** ppBinding,
    SQLSMALLINT* pDisplay);


void DisplayTitles(HSTMT    hStmt,
    DWORD    cDisplaySize,
    BINDING* pBinding);

void SetConsole(DWORD   cDisplaySize,
    BOOL    fInvert);

/*****************************************/
/* Some constants                        */
/*****************************************/


#define DISPLAY_MAX 50          // Arbitrary limit on column width to display
#define DISPLAY_FORMAT_EXTRA 3  // Per column extra display bytes (| <data> )
#define DISPLAY_FORMAT      L"%c %*.*s "
#define DISPLAY_FORMAT_C    L"%c %-*.*s "
#define NULL_SIZE           6   // <NULL>
#define SQL_QUERY_SIZE      1000 // Max. Num characters for SQL Query passed in.

#define PIPE                L'|'

SHORT   gHeight = 80;       // Users screen height



void convert(string szSource, WCHAR* wszDest)
{
    const size_t WCHARBUF = 1000;
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szSource.c_str(), -1, wszDest, WCHARBUF);
}


void ExecuteSQLStmt(WCHAR* wszInput, SQLHSTMT& hStmt)
{

    RETCODE     RetCode;
    SQLSMALLINT sNumResults;


    RetCode = SQLExecDirect(hStmt, wszInput, SQL_NTS);

    switch (RetCode)
    {
    case SQL_SUCCESS_WITH_INFO:
    {
        HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
        // fall through
    }
    case SQL_SUCCESS:
    {
        // If this is a row-returning query, display
        // results
        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLNumResultCols(hStmt, &sNumResults));

        if (sNumResults > 0)
        {
            DisplayResults(hStmt, sNumResults);
        }
        else
        {
            SQLLEN cRowCount;

            TRYODBC(hStmt,
                SQL_HANDLE_STMT,
                SQLRowCount(hStmt, &cRowCount));

            if (cRowCount >= 0)
            {
                wprintf(L"%Id %s affected\n",
                    cRowCount,
                    cRowCount == 1 ? L"row" : L"rows");
            }
        }
        break;
    }

    case SQL_ERROR:
    {
        HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
        break;
    }

    default:
        fwprintf(stderr, L"Unexpected return code %hd!\n", RetCode);

    }
    TRYODBC(hStmt,
        SQL_HANDLE_STMT,
        SQLFreeStmt(hStmt, SQL_CLOSE));

Exit:
    ;
}

int __cdecl wmain(int argc, _In_reads_(argc) WCHAR** argv)
{
    SQLHENV     hEnv = NULL;
    SQLHDBC     hDbc = NULL;
    SQLHSTMT    hStmt = NULL;
    WCHAR* pwszConnStr = NULL;
    WCHAR       wszInput[SQL_QUERY_SIZE];

    //Students should modifiy the following variables
    string D_ID = "";
    string D_Name = "";
    string Phone_Number = "";
    string Address = "";
    string Salary = "";
    string Field = "";
    string P_Name = "";
    string PPhone_Number = "";
    string P_ID = "";
    string Email = "";
    string Sex = "";
    string Appointment_id = "";
    string Booked_Date = "";
    string Patient_Description = "";
    string N_ID = "";
    string N_Name = "";
    string Phone_Number_ = "";
    string nSalary = "";
    string Address_ = "";

    //Medical info
    string Blood_type = "";
    string Animia = "";
    string Blood_Disorder = "";
    string Cancer = "";
    string Depression = "";
    string Kidney_Disease = "";
    string Lung_Disease = "";
    string Heart_Problems = "";
    string Thyroid_Disease = "";
    string HIV = "";
    string Covid_19 = "";
    string Pregnant = "";
    string Diabetes = "";
    string Blood_Pressure = "";

    //Bill Info
    string discount = "";
    string bill_charges = "";
    string bill_no = "";


    // Allocate an environment

    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_ERROR)
    {
        fwprintf(stderr, L"Unable to allocate an environment handle\n");
        exit(-1);
    }

    // Register this as an application that expects 3.x behavior,
    // you must register something if you use AllocHandle

    TRYODBC(hEnv,
        SQL_HANDLE_ENV,
        SQLSetEnvAttr(hEnv,
            SQL_ATTR_ODBC_VERSION,
            (SQLPOINTER)SQL_OV_ODBC3,
            0));

    // Allocate a connection
    TRYODBC(hEnv,
        SQL_HANDLE_ENV,
        SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc));

    if (argc > 1)
    {
        pwszConnStr = *++argv;
    }
    else
    {
        pwszConnStr = L"";
    }


    //Student must insert his/her connection string
    pwszConnStr = L"Driver={SQL Server};Server=DB221341.mssql.somee.com;Database=DB221341;Uid=A221341_SQLLogin_1;Pwd=ex4yhk8dda;";
    //pwszConnStr = L"Driver={SQL Server};Server=localhost\\SQLEXPRESS;Database=Company;Trusted_Connection=Yes;";

    // Connect to the driver.  Use the connection string if supplied
    // on the input, otherwise let the driver manager prompt for input.

    TRYODBC(hDbc,
        SQL_HANDLE_DBC,
        SQLDriverConnect(hDbc,
            GetDesktopWindow(),
            pwszConnStr,
            SQL_NTS,
            NULL,
            0,
            NULL,
            SQL_DRIVER_COMPLETE));

    fwprintf(stderr, L"Connected!\n");

    TRYODBC(hDbc,
        SQL_HANDLE_DBC,
        SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt));


    //User Code
    int choice = 10;

    while (choice != -1)
    {
        int doctorChoice = 10;
        int patientChoice = 10;
        int appointmentChoice = 10;
        int nurseChoice = 10;
        int medChoice = 10;
        int pat_appChoice = 10;
        int billChoice = 10;
        int assistChoice = 10;

        cout << "==============================" << endl;
        cout << "=   Welcome to Health Care   =" << endl;
        cout << "==============================" << endl;
        cout << "==============================" << endl;
        cout << "=         Main Menu          =" << endl;
        cout << "==============================" << endl;
        cout << "1: Doctors Menu" << endl;
        cout << "2: Patient Menu" << endl;
        cout << "3: Appointments Menu" << endl;
        cout << "4: Nurses Menu" << endl;
        cout << "5: Medical Info Menu" << endl;
        cout << "6: Patient Appointment Info Menu" << endl;
        cout << "7: Bill Info Menu" << endl;
        cout << "8: Assists Menu" << endl;
        cout << "-1: Exit" << endl;
        cout << "Choice: ";
        cin >> choice;
        switch (choice)
        {
        case 1:
            while (doctorChoice != -1)
            {
                cout << "==============================" << endl;
                cout << "=       Doctors Menu        = " << endl;
                cout << "==============================" << endl;
                cout << "1: Show All Doctors" << endl;
                cout << "2: Insert new Doctor" << endl;
                cout << "3: Delete current Doctor" << endl;
                cout << "4: Update current Doctor" << endl;
                cout << "5: Display Doctors' salary in descending order" << endl;
                cout << "-1: Back to the main menu" << endl;
                cout << "Choice: ";
                cin >> doctorChoice;
                switch (doctorChoice)
                {
                case 1:
                    cout << "==============================" << endl;
                    convert("Select * from Doctors", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                case 2:
                    cout << "==============================" << endl;
                    cout << "Enter Doctors ID: ";
                    cin >> D_ID;
                    cout << "Enter Doctors name: ";
                    cin >> D_Name;
                    cout << "Enter Doctors phone number: ";
                    cin >> Phone_Number;
                    cout << "Enter Doctors address: ";
                    cin >> Address;
                    cout << "Enter Doctors salary: ";
                    cin >> Salary;
                    cout << "Enter Doctors field: ";
                    cin >> Field;

                    convert("insert into Doctors values(" + D_ID + ",'" + D_Name + "','" + Phone_Number + "','" + Address + "','" + Salary + "','" + Field + "')", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 3:
                    cout << "==============================" << endl;
                    cout << "Enter Doctor ID to delete: ";
                    cin >> D_ID;
                    convert("delete from Doctors where D_ID = " + D_ID, wszInput);
                    //cout << "delete from Students where id = " + D_ID << endl;
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 4:
                    cout << "==============================" << endl;
                    cout << "=       Doctors Update Menu        = " << endl;
                    cout << "==============================" << endl;
                    cout << "1: Update Doctor Name" << endl;
                    cout << "2: Update Doctor address" << endl;
                    cout << "3: Update Doctor salary" << endl;
                    cout << "4: Update Doctor's Field" << endl;
                    cout << "-1: Back to the main menu" << endl;
                    cin >> doctorChoice;
                    switch (doctorChoice)
                    {
                    case 1:
                        cout << "Enter Doctor ID: ";
                        cin >> D_ID;
                        cout << "Enter Doctor name: ";
                        cin >> D_Name;
                        convert("update Doctors set D_Name = '" + D_Name + "' where D_ID = " + D_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 2:
                        cout << "Enter Doctor ID: ";
                        cin >> D_ID;
                        cout << "Enter Doctor Phone number: ";
                        cin >> Phone_Number;
                        convert("update Doctors set Phone_Number = '" + Phone_Number + "' where D_ID = " + D_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 3:
                        cout << "Enter Doctor ID: ";
                        cin >> D_ID;
                        cout << "Enter Doctor Address: ";
                        cin >> Address;
                        convert("update Doctors set Address_ = '" + Address + "' where D_ID = " + D_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 4:
                        cout << "Enter Doctor ID: ";
                        cin >> D_ID;
                        cout << "Enter Doctor field: ";
                        cin >> Field;
                        convert("update Doctors set Field  = '" + Field + "' where D_ID = " + D_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    }
                case 5:
                    cout << "==============================" << endl;
                    convert("Select * from Doctors ORDER BY Salary DESC", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                } 
            }
            break;
        case 2:
            while (patientChoice != -1)
            {
                cout << "==============================" << endl;
                cout << "=       Patients Menu        =" << endl;
                cout << "==============================" << endl;
                cout << "1: Show All Patients" << endl;
                cout << "2: Insert new Patient" << endl;
                cout << "3: Delete current Patient" << endl;
                cout << "4: Update current Patient" << endl;
                cout << "-1: Back to the main menu" << endl;
                cout << "Choice: ";
                cin >> patientChoice;
                switch (patientChoice)
                {
                case 1:
                    cout << "==============================" << endl;
                    convert("Select * from Patient", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                case 2:
                    cout << "==============================" << endl;
                    cout << "Enter Patient id: ";
                    cin >> P_ID;
                    cout << "Enter Patient name: ";
                    cin >> P_Name;
                    cout << "Enter Patient phone number: ";
                    cin >> PPhone_Number;
                    cout << "Enter Patient email: ";
                    cin >> Email;
                    cout << "Enter Patient sex: ";
                    cin >> Sex;
                    convert("insert into Patient values('" + P_Name + "','" + PPhone_Number + "','" + P_ID + "','" + Email + "','" + Sex + "')", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 3:
                    cout << "==============================" << endl;
                    cout << "Enter Patient ID to delete: ";
                    cin >> P_ID;
                    convert("delete from Patient where P_ID = '" + P_ID + "'", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 4:
                    cout << "==============================" << endl;
                    cout << "=    Patients Update Menu    = " << endl;
                    cout << "==============================" << endl;
                    cout << "1: Update Patient's Name" << endl;
                    cout << "2: Update Patient's Phone Number" << endl;
                    cout << "3: Update Patient's Email" << endl;
                    cout << "4: Update Patient's Sex" << endl;
                    cout << "-1: Back to the main menu" << endl;
                    cin >> patientChoice;
                    switch (patientChoice)
                    {
                    case 1:
                        cout << "Enter Patient ID to update: ";
                        cin >> P_ID;
                        cout << "Enter Patient name to update: ";
                        cin >> P_Name;
                        convert("update Patient set P_Name = '" + P_Name + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 2:
                        cout << "Enter Patient ID to update: ";
                        cin >> P_ID;
                        cout << "Enter the Patient's new phone number to update: ";
                        cin >> PPhone_Number;
                        convert("update Patient set Phone_Number = '" + PPhone_Number + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 3:
                        cout << "Enter Patient ID to update: ";
                        cin >> P_ID;
                        cout << "Enter Patient's new email to update: ";
                        cin >> Email;
                        convert("update Patient set Email = '" + Email + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 4:
                        cout << "Enter Patient ID to update: ";
                        cin >> P_ID;
                        cout << "Enter Patient sex to update: ";
                        cin >> Sex;
                        convert("update Patient set Sex  = '" + Sex + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    }
                    break;
                }
            }
            break;
        case 3:
            while (appointmentChoice != -1)
            {
                cout << "==============================" << endl;
                cout << "=       Patients Menu        =" << endl;
                cout << "==============================" << endl;
                cout << "1: Show All Appointments" << endl;
                cout << "2: Insert new Appointment" << endl;
                cout << "3: Delete current Appointment" << endl;
                cout << "4: Update current Appointment" << endl;
                cout << "-1: Back to the main menu" << endl;
                cout << "Choice: ";
                cin >> appointmentChoice;
                switch (appointmentChoice)
                {
                case 1:
                    cout << "==============================" << endl;
                    convert("Select * from Appointments", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                case 2:
                    cout << "==============================" << endl;
                    cout << "Enter Appointment_id: ";
                    cin >> Appointment_id;
                    cout << "Enter Booked_Date: ";
                    cin >> Booked_Date;
                    cout << "Enter Patient_Description: ";
                    cin >> Patient_Description;
                    convert("insert into Appointments values('" + Appointment_id + "','" + Booked_Date + "','" + Patient_Description + "')", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 3:
                    cout << "==============================" << endl;
                    cout << "Enter Appointment_id to delete: ";
                    cin >> Appointment_id;
                    convert("delete from Appointments where Appointment_id = '" + Appointment_id + "'", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 4:
                    cout << "==============================" << endl;
                    cout << "Enter Appointment_id TO UPDATE: ";
                    cin >> Appointment_id;
                    cout << "Enter Booked_Date TO UPDATE: ";
                    cin >> Booked_Date;
                    cout << "Enter Patient_Description TO UPDATE: ";
                    cin >> Patient_Description;
                    convert("update Appointments set Booked_Date = '" + Booked_Date + "' where Appointment_id = " + Appointment_id, wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    convert("update Appointments set Patient_Description = '" + Patient_Description + "' where Appointment_id = " + Appointment_id, wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                }
            }
            break;
        case 4:
            while (nurseChoice != -1)
            {
                cout << "==============================" << endl;
                cout << "=       Nurses Menu        =" << endl;
                cout << "==============================" << endl;
                cout << "1: Show All Nurses" << endl;
                cout << "2: Insert new Nurses" << endl;
                cout << "3: Delete current Nurses" << endl;
                cout << "4: Update current Nurses" << endl;
                cout << "-1: Back to the main menu" << endl;
                cout << "Choice: ";
                cin >> nurseChoice;
                switch (nurseChoice)
                {
                case 1:
                    cout << "==============================" << endl;
                    convert("Select * from Nurses", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                case 2:
                    cout << "==============================" << endl;
                    cout << "Enter Nurse_id: ";
                    cin >> N_ID;
                    cout << "Enter Nurse name: ";
                    cin >> N_Name;
                    cout << "Enter Nurse phone number: ";
                    cin >> Phone_Number_;
                    cout << "Enter Nurse Salary: ";
                    cin >> nSalary;
                    cout << "Enter Nurse Address: ";
                    cin >> Address_;
                    convert("insert into Nurses values('" + N_ID + "','" + N_Name + "','" + Phone_Number_ + "','" + nSalary + "','" + Address_ + "')", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 3:
                    cout << "==============================" << endl;
                    cout << "Enter Nurse ID to delete: ";
                    cin >> N_ID;
                    convert("delete from Nurses where N_ID = '" + N_ID + "'", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 4:
                    cout << "==============================" << endl;
                    cout << "=     Nurses Update Menu     = " << endl;
                    cout << "==============================" << endl;
                    cout << "1: Update Nurse's Name" << endl;
                    cout << "2: Update Nurse's Phone Number" << endl;
                    cout << "3: Update Nurse's Salary" << endl;
                    cout << "4: Update Nurse's Address" << endl;
                    cout << "-1: Back to the main menu" << endl;
                    cin >> nurseChoice;
                    switch (nurseChoice)
                    {
                    case 1:
                        cout << "Enter Nurse_id to update: ";
                        cin >> N_ID;
                        cout << "Enter Nurse name to update: ";
                        cin >> N_Name;
                        convert("update Nurses set N_Name = '" + N_Name + "' where N_ID = " + N_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 2:
                        cout << "Enter Nurse_id to update: ";
                        cin >> N_ID;
                        cout << "Enter Nurse phone number to update: ";
                        cin >> Phone_Number_;
                        convert("update Nurses set Phone_Number_ = '" + Phone_Number_ + "' where N_ID = " + N_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 3:
                        cout << "Enter Nurse_id to update: ";
                        cin >> N_ID;
                        cout << "Enter Nurse Salary to update: ";
                        cin >> nSalary;
                        convert("update Nurses set Salary = '" + nSalary + "' where N_ID = " + N_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 4:
                        cout << "Enter Nurse_id to update: ";
                        cin >> N_ID;
                        cout << "Enter Nurse Address to update: ";
                        cin >> Address_;
                        convert("update Nurses set Address_  = '" + Address_ + "' where N_ID = " + N_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    }
                }
            }
            break;
        case 5:
            while (medChoice != -1)
            {
                cout << "==============================" << endl;
                cout << "=       Medical Info         =" << endl;
                cout << "==============================" << endl;
                cout << "1: Show All Medical Info" << endl;
                cout << "2: Insert new Medical Info" << endl;
                cout << "3: Delete current Medical Info" << endl;
                cout << "4: Update current Medical Info" << endl;
                cout << "-1: Back to the main menu" << endl;
                cout << "Choice: ";
                cin >> medChoice;
                switch (medChoice)
                {
                case 1:
                    cout << "==============================" << endl;
                    convert("Select * from Medical_info", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                case 2:
                    cout << "==============================" << endl;
                    cout << "Enter Patient ID: ";
                    cin >> P_ID;
                    cout << "Animia: ";
                    cin >> Animia;
                    cout << "Kidney_Disease: ";
                    cin >> Kidney_Disease;
                    cout << "Lung_Disease: ";
                    cin >> Lung_Disease;
                    cout << "Heart_Problems: ";
                    cin >> Heart_Problems;
                    cout << "Thyroid_Disease: ";
                    cin >> Thyroid_Disease;
                    cout << "HIV: ";
                    cin >> HIV;
                    cout << "Covid_19: ";
                    cin >> Covid_19;
                    cout << "Pregnant: ";
                    cin >> Pregnant;
                    cout << "Diabetes: ";
                    cin >> Diabetes;
                    cout << "Blood_Pressure: ";
                    cin >> Blood_Pressure;
                    convert("insert into Medical_info values('" + Animia + "','" + Kidney_Disease + "','" + Lung_Disease + "','" + Heart_Problems + "','" + Thyroid_Disease + "', '" + HIV + "' ,'" + Covid_19 + "' , '" + Pregnant + "','" + Diabetes + "','" + Blood_Pressure + "', '" + P_ID + "')", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl; 
                    break;
                case 3:
                    cout << "==============================" << endl;
                    cout << "Enter Patient ID to delete: ";
                    cin >> P_ID;
                    convert("delete from Medical_info where P_ID = '" + P_ID + "'", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 4:
                    cout << "==============================" << endl;
                    cout << "=  Medical info Update Menu  =" << endl;
                    cout << "==============================" << endl;
                    cout << "1: Update Animia Status" << endl;
                    cout << "2: Update Kidney Disease Status" << endl;
                    cout << "3: Update Lung Disease Status" << endl;
                    cout << "4: Update Heart Problems Status" << endl;
                    cout << "5: Update Thyroid Disease Status" << endl;
                    cout << "6: Update HIV Status" << endl;
                    cout << "7: Update Covid 19 Status" << endl;
                    cout << "8: Update Pregnant Status" << endl;
                    cout << "9: Update Diabetes Status" << endl;
                    cout << "10: Update Blood Pressure Status" << endl;
                    cout << "-1: Back to the main menu" << endl;
                    cin >> medChoice;
                    switch (medChoice)
                    {
                    case 1:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Animia: ";
                        cin >> Animia;
                        convert("update Medical_info set Animia  = '" + Animia + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 2:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Kidney_Disease: ";
                        cin >> Kidney_Disease;
                        convert("update Medical_info set Kidney_Disease = '" + Kidney_Disease + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 3:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Lung_Disease: ";
                        cin >> Lung_Disease;
                        convert("update Medical_info set Lung_Disease = '" + Lung_Disease + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 4:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Heart_Problems: ";
                        cin >> Heart_Problems;
                        convert("update Medical_info set Heart_Problems  = '" + Heart_Problems + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 5:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Thyroid_Disease: ";
                        cin >> Thyroid_Disease;
                        convert("update Medical_info set Thyroid_Disease  = '" + Thyroid_Disease + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 6:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "HIV: ";
                        cin >> HIV;
                        convert("update Medical_info set HIV  = '" + HIV + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 7:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Covid_19: ";
                        cin >> Covid_19;
                        convert("update Medical_info set Covid_19  = '" + Covid_19 + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 8:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Pregnant: ";
                        cin >> Pregnant;
                        convert("update Medical_info set Pregnant  = '" + Pregnant + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 9:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Diabetes: ";
                        cin >> Diabetes;
                        convert("update Medical_info set Diabetes  = '" + Diabetes + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 10:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Blood_Pressure: ";
                        cin >> Blood_Pressure;
                        convert("update Medical_info set Blood_Pressure  = '" + Blood_Pressure + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    }
                    break;
                }
            }
            break;
        case 6:
            while (pat_appChoice != -1)
            {
                cout << "==============================" << endl;
                cout << "=       PAT_APP Menu        =" << endl;
                cout << "==============================" << endl;
                cout << "1: Show All Patient Appointments" << endl;
                cout << "2: Insert new Patient Appointment" << endl;
                cout << "3: Delete current Patient Appointment" << endl;
                cout << "4: Update current Patient Appointment" << endl;
                cout << "-1: Back to the main menu" << endl;
                cout << "Choice: ";
                cin >> pat_appChoice;
                switch (pat_appChoice)
                {
                case 1:
                    cout << "==============================" << endl;
                    convert("Select * from PAT_APP", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                case 2:
                    cout << "==============================" << endl;
                    cout << "Enter Patient ID: ";
                    cin >> P_ID;
                    cout << "Enter Appointment ID: ";
                    cin >> Appointment_id;
                    convert("insert into PAT_APP values('" + P_ID + "','" + Appointment_id + "')", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 3:
                    cout << "==============================" << endl;
                    cout << "Enter Patient ID to delete Appointment: ";
                    cin >> P_ID;
                    convert("delete from PAT_APP where P_ID = '" + P_ID + "'", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 4:
                    cout << "==============================" << endl;
                    cout << "Enter Patient ID: ";
                    cin >> P_ID;
                    cout << "Enter Appointment ID: ";
                    cin >> Appointment_id;
                    convert("update PAT_APP set Appointment_id = '" + Appointment_id + "' where P_ID = " + P_ID, wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                }
            }
            break;
        case 7:
            while (billChoice != -1)
            {
                cout << "==============================" << endl;
                cout << "=       Bill Menu        =" << endl;
                cout << "==============================" << endl;
                cout << "1: Show All Bills" << endl;
                cout << "2: Insert new Bill" << endl;
                cout << "3: Delete current Bill" << endl;
                cout << "4: Update current Bill" << endl;
                cout << "5: Display Bills in descending order" << endl;
                cout << "-1: Back to the main menu" << endl;
                cout << "Choice: ";
                cin >> billChoice;
                switch (billChoice)
                {
                case 1:
                    cout << "==============================" << endl;
                    convert("Select * from bill", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                case 2:
                    cout << "==============================" << endl;
                    cout << "Enter Patient ID: ";
                    cin >> P_ID;
                    cout << "Enter Discount: ";
                    cin >> discount;
                    cout << "Enter Bill Charges: ";
                    cin >> bill_charges;
                    cout << "Enter Bill Number: ";
                    cin >> bill_no;
                    convert("insert into bill values('" + discount + "','" + bill_charges + "', '" + bill_no + "', '" + P_ID + "')", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 3:
                    cout << "==============================" << endl;
                    cout << "Enter Patient ID to delete Appointment: ";
                    cin >> P_ID;
                    convert("delete from bill where P_ID = '" + P_ID + "'", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 4:
                    cout << "==============================" << endl;
                    cout << "=     Bill Update Menu     = " << endl;
                    cout << "==============================" << endl;
                    cout << "1: Update Bill Number" << endl;
                    cout << "2: Update Bill Charges" << endl;
                    cout << "3: Update Discounts" << endl;
                    cout << "-1: Back to the main menu" << endl;
                    cin >> billChoice;
                    switch (billChoice)
                    {
                    case 1:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Enter Bill Number: ";
                        cin >> bill_no;
                        convert("update bill set bill_no = '" + bill_no + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 2:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Enter Bill Charges: ";
                        cin >> bill_charges;
                        convert("update bill set bill_charges = '" + bill_charges + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    case 3:
                        cout << "Enter Patient ID: ";
                        cin >> P_ID;
                        cout << "Enter Discount: ";
                        cin >> discount;
                        convert("update bill set discount = '" + discount + "' where P_ID = " + P_ID, wszInput);
                        ExecuteSQLStmt(wszInput, hStmt);
                        cout << "Changes has been saved successfully!" << endl;
                        break;
                    }
                case 5:
                    cout << "==============================" << endl;
                    convert("Select * from bill ORDER BY bill_charges DESC", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                }        
            }
            break;
        case 8:
            while (assistChoice != -1)
            {
                cout << "==============================" << endl;
                cout << "=       Assist Menu        =" << endl;
                cout << "==============================" << endl;
                cout << "1: Show All Assists" << endl;
                cout << "2: Insert new Assist" << endl;
                cout << "3: Delete current Assist" << endl;
                cout << "4: Update current Assist" << endl;
                cout << "-1: Back to the main menu" << endl;
                cout << "Choice: ";
                cin >> assistChoice;
                switch (assistChoice)
                {
                case 1:
                    cout << "==============================" << endl;
                    convert("Select * from Assists", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    break;
                case 2:
                    cout << "==============================" << endl;
                    cout << "Enter Doctor ID: ";
                    cin >> D_ID;
                    cout << "Enter Nurse ID: ";
                    cin >> N_ID;
                    convert("insert into Assists values('" + D_ID + "','" + N_ID + "')", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 3:
                    cout << "==============================" << endl;
                    cout << "Enter Patient ID to delete Appointment: ";
                    cin >> P_ID;
                    convert("delete from Assists where D_ID = '" + D_ID + "'", wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                case 4:
                    cout << "==============================" << endl;
                    cout << "Enter Doctor ID: ";
                    cin >> D_ID;
                    cout << "Enter Nurse ID: ";
                    cin >> N_ID;
                    convert("update Assists set N_ID = '" + N_ID + "' where D_ID = " + D_ID, wszInput);
                    ExecuteSQLStmt(wszInput, hStmt);
                    cout << "Changes has been saved successfully!" << endl;
                    break;
                }
            }
            break;
        }
    }

Exit:

    // Free ODBC handles and exit

    if (hStmt)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    if (hDbc)
    {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    }

    if (hEnv)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }

    wprintf(L"\nDisconnected.");

    return 0;

}

/************************************************************************
/* DisplayResults: display results of a select query
/*
/* Parameters:
/*      hStmt      ODBC statement handle
/*      cCols      Count of columns
/************************************************************************/

void DisplayResults(HSTMT       hStmt,
    SQLSMALLINT cCols)
{
    BINDING* pFirstBinding, * pThisBinding;
    SQLSMALLINT     cDisplaySize;
    RETCODE         RetCode = SQL_SUCCESS;
    int             iCount = 0;

    // Allocate memory for each column 

    AllocateBindings(hStmt, cCols, &pFirstBinding, &cDisplaySize);

    // Set the display mode and write the titles

    DisplayTitles(hStmt, cDisplaySize + 1, pFirstBinding);


    // Fetch and display the data

    bool fNoData = false;

    do {
        // Fetch a row

        if (iCount++ >= gHeight - 2)
        {
            int     nInputChar;
            bool    fEnterReceived = false;

            while (!fEnterReceived)
            {
                wprintf(L"              ");
                SetConsole(cDisplaySize + 2, TRUE);
                wprintf(L"   Press ENTER to continue, Q to quit (height:%hd)", gHeight);
                SetConsole(cDisplaySize + 2, FALSE);

                nInputChar = _getch();
                wprintf(L"\n");
                if ((nInputChar == 'Q') || (nInputChar == 'q'))
                {
                    goto Exit;
                }
                else if ('\r' == nInputChar)
                {
                    fEnterReceived = true;
                }
                // else loop back to display prompt again
            }

            iCount = 1;
            DisplayTitles(hStmt, cDisplaySize + 1, pFirstBinding);
        }

        TRYODBC(hStmt, SQL_HANDLE_STMT, RetCode = SQLFetch(hStmt));

        if (RetCode == SQL_NO_DATA_FOUND)
        {
            fNoData = true;
        }
        else
        {

            // Display the data.   Ignore truncations

            for (pThisBinding = pFirstBinding;
                pThisBinding;
                pThisBinding = pThisBinding->sNext)
            {
                if (pThisBinding->indPtr != SQL_NULL_DATA)
                {
                    wprintf(pThisBinding->fChar ? DISPLAY_FORMAT_C : DISPLAY_FORMAT,
                        PIPE,
                        pThisBinding->cDisplaySize,
                        pThisBinding->cDisplaySize,
                        pThisBinding->wszBuffer);
                }
                else
                {
                    wprintf(DISPLAY_FORMAT_C,
                        PIPE,
                        pThisBinding->cDisplaySize,
                        pThisBinding->cDisplaySize,
                        L"<NULL>");
                }
            }
            wprintf(L" %c\n", PIPE);
        }
    } while (!fNoData);

    SetConsole(cDisplaySize + 2, TRUE);
    wprintf(L"%*.*s", cDisplaySize + 2, cDisplaySize + 2, L" ");
    SetConsole(cDisplaySize + 2, FALSE);
    wprintf(L"\n");

Exit:
    // Clean up the allocated buffers

    while (pFirstBinding)
    {
        pThisBinding = pFirstBinding->sNext;
        free(pFirstBinding->wszBuffer);
        free(pFirstBinding);
        pFirstBinding = pThisBinding;
    }
}

/************************************************************************
/* AllocateBindings:  Get column information and allocate bindings
/* for each column.
/*
/* Parameters:
/*      hStmt      Statement handle
/*      cCols       Number of columns in the result set
/*      *lppBinding Binding pointer (returned)
/*      lpDisplay   Display size of one line
/************************************************************************/

void AllocateBindings(HSTMT         hStmt,
    SQLSMALLINT   cCols,
    BINDING** ppBinding,
    SQLSMALLINT* pDisplay)
{
    SQLSMALLINT     iCol;
    BINDING* pThisBinding, * pLastBinding = NULL;
    SQLLEN          cchDisplay, ssType;
    SQLSMALLINT     cchColumnNameLength;

    *pDisplay = 0;

    for (iCol = 1; iCol <= cCols; iCol++)
    {
        pThisBinding = (BINDING*)(malloc(sizeof(BINDING)));
        if (!(pThisBinding))
        {
            fwprintf(stderr, L"Out of memory!\n");
            exit(-100);
        }

        if (iCol == 1)
        {
            *ppBinding = pThisBinding;
        }
        else
        {
            pLastBinding->sNext = pThisBinding;
        }
        pLastBinding = pThisBinding;


        // Figure out the display length of the column (we will
        // bind to char since we are only displaying data, in general
        // you should bind to the appropriate C type if you are going
        // to manipulate data since it is much faster...)

        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLColAttribute(hStmt,
                iCol,
                SQL_DESC_DISPLAY_SIZE,
                NULL,
                0,
                NULL,
                &cchDisplay));


        // Figure out if this is a character or numeric column; this is
        // used to determine if we want to display the data left- or right-
        // aligned.

        // SQL_DESC_CONCISE_TYPE maps to the 1.x SQL_COLUMN_TYPE. 
        // This is what you must use if you want to work
        // against a 2.x driver.

        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLColAttribute(hStmt,
                iCol,
                SQL_DESC_CONCISE_TYPE,
                NULL,
                0,
                NULL,
                &ssType));


        pThisBinding->fChar = (ssType == SQL_CHAR ||
            ssType == SQL_VARCHAR ||
            ssType == SQL_LONGVARCHAR);

        pThisBinding->sNext = NULL;

        // Arbitrary limit on display size
        if (cchDisplay > DISPLAY_MAX)
            cchDisplay = DISPLAY_MAX;

        // Allocate a buffer big enough to hold the text representation
        // of the data.  Add one character for the null terminator

        pThisBinding->wszBuffer = (WCHAR*)malloc((cchDisplay + 1) * sizeof(WCHAR));

        if (!(pThisBinding->wszBuffer))
        {
            fwprintf(stderr, L"Out of memory!\n");
            exit(-100);
        }

        // Map this buffer to the driver's buffer.   At Fetch time,
        // the driver will fill in this data.  Note that the size is 
        // count of bytes (for Unicode).  All ODBC functions that take
        // SQLPOINTER use count of bytes; all functions that take only
        // strings use count of characters.

        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLBindCol(hStmt,
                iCol,
                SQL_C_TCHAR,
                (SQLPOINTER)pThisBinding->wszBuffer,
                (cchDisplay + 1) * sizeof(WCHAR),
                &pThisBinding->indPtr));


        // Now set the display size that we will use to display
        // the data.   Figure out the length of the column name

        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLColAttribute(hStmt,
                iCol,
                SQL_DESC_NAME,
                NULL,
                0,
                &cchColumnNameLength,
                NULL));

        pThisBinding->cDisplaySize = max((SQLSMALLINT)cchDisplay, cchColumnNameLength);
        if (pThisBinding->cDisplaySize < NULL_SIZE)
            pThisBinding->cDisplaySize = NULL_SIZE;

        *pDisplay += pThisBinding->cDisplaySize + DISPLAY_FORMAT_EXTRA;

    }

    return;

Exit:

    exit(-1);

    return;
}


/************************************************************************
/* DisplayTitles: print the titles of all the columns and set the
/*                shell window's width
/*
/* Parameters:
/*      hStmt          Statement handle
/*      cDisplaySize   Total display size
/*      pBinding        list of binding information
/************************************************************************/

void DisplayTitles(HSTMT     hStmt,
    DWORD     cDisplaySize,
    BINDING* pBinding)
{
    WCHAR           wszTitle[DISPLAY_MAX];
    SQLSMALLINT     iCol = 1;

    SetConsole(cDisplaySize + 2, TRUE);

    for (; pBinding; pBinding = pBinding->sNext)
    {
        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLColAttribute(hStmt,
                iCol++,
                SQL_DESC_NAME,
                wszTitle,
                sizeof(wszTitle), // Note count of bytes!
                NULL,
                NULL));

        wprintf(DISPLAY_FORMAT_C,
            PIPE,
            pBinding->cDisplaySize,
            pBinding->cDisplaySize,
            wszTitle);
    }

Exit:

    wprintf(L" %c", PIPE);
    SetConsole(cDisplaySize + 2, FALSE);
    wprintf(L"\n");

}


/************************************************************************
/* SetConsole: sets console display size and video mode
/*
/*  Parameters
/*      siDisplaySize   Console display size
/*      fInvert         Invert video?
/************************************************************************/

void SetConsole(DWORD dwDisplaySize,
    BOOL  fInvert)
{
    HANDLE                          hConsole;
    CONSOLE_SCREEN_BUFFER_INFO      csbInfo;

    // Reset the console screen buffer size if necessary

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hConsole != INVALID_HANDLE_VALUE)
    {
        if (GetConsoleScreenBufferInfo(hConsole, &csbInfo))
        {
            if (csbInfo.dwSize.X < (SHORT)dwDisplaySize)
            {
                csbInfo.dwSize.X = (SHORT)dwDisplaySize;
                SetConsoleScreenBufferSize(hConsole, csbInfo.dwSize);
            }

            gHeight = csbInfo.dwSize.Y;
        }

        if (fInvert)
        {
            SetConsoleTextAttribute(hConsole, (WORD)(csbInfo.wAttributes | BACKGROUND_BLUE));
        }
        else
        {
            SetConsoleTextAttribute(hConsole, (WORD)(csbInfo.wAttributes & ~(BACKGROUND_BLUE)));
        }
    }
}


/************************************************************************
/* HandleDiagnosticRecord : display error/warning information
/*
/* Parameters:
/*      hHandle     ODBC handle
/*      hType       Type of handle (HANDLE_STMT, HANDLE_ENV, HANDLE_DBC)
/*      RetCode     Return code of failing command
/************************************************************************/

void HandleDiagnosticRecord(SQLHANDLE      hHandle,
    SQLSMALLINT    hType,
    RETCODE        RetCode)
{
    SQLSMALLINT iRec = 0;
    SQLINTEGER  iError;
    WCHAR       wszMessage[1000];
    WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];


    if (RetCode == SQL_INVALID_HANDLE)
    {
        fwprintf(stderr, L"Invalid handle!\n");
        return;
    }

    while (SQLGetDiagRec(hType,
        hHandle,
        ++iRec,
        wszState,
        &iError,
        wszMessage,
        (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)),
        (SQLSMALLINT*)NULL) == SQL_SUCCESS)
    {
        // Hide data truncated..
        if (wcsncmp(wszState, L"01004", 5))
        {
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
        }
    }

}
