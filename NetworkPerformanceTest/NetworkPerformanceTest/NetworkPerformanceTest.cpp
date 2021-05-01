// NetworkPerformanceTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <fstream>

#include <Windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <ctime>
#include <vector>
#include "CArgParser.h"

using namespace std;

struct threadTestParam{
	int FileMB;
	string TargetFilePath;
	
	struct Result{
		int mode;
		int threadId;
		string fileName;
		double durationInSeconds;
		long fileSizeByte;
		int fileSizeMB;
		time_t startTime;
		time_t endTime;

	} result, read_result;

	
};

struct fileInfo {
	long FileSize;
	string FileName;
};

string convertTime2Str(time_t param){

	time_t tm = param;
	wchar_t buf[26];
	errno_t err;
	
	//ctime_s(dt, sizeof(dt), &tm);
	err = _wctime_s(buf, 26, &tm);
	if (err != 0)
	{
		printf("Invalid Arguments for _wctime_s. Error Code: %d\n", err);
	}
	wstring ws(buf);
	string temp (ws.begin(),ws.end()-1); //chop the \n at the end
	return temp;

}

string GetCurrentPath() 
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}

string OutputCsv(vector<threadTestParam> paramLst, string sFName)
{
		
	
	string sCsvFile = GetCurrentPath() + "\\" + sFName;
	ofstream fsCsvOutput;
	fsCsvOutput.open(sCsvFile);
	
	fsCsvOutput << "Mode,Thread Id,Output File, WRITE Duration (Seconds),WRITE Start,WRITE End,WRITE Bytes,WRITE MB"; //header 1
	fsCsvOutput << ",READ Duration (Seconds),READ Start,READ End,READ Bytes,READ MB\n"; //header 2
	for (int i = 0; i < (int)paramLst.size(); i++){

		fsCsvOutput << paramLst[i].result.mode << ",";

		string startTime = convertTime2Str(paramLst[i].result.startTime);
		string endTime = convertTime2Str(paramLst[i].result.endTime);
		fsCsvOutput << paramLst[i].result.threadId << ",";
		fsCsvOutput << paramLst[i].result.fileName.c_str() << ",";
		
		fsCsvOutput << paramLst[i].result.durationInSeconds << ",";
		fsCsvOutput << startTime.c_str() << ",";
		fsCsvOutput << endTime.c_str() << ",";
		fsCsvOutput << paramLst[i].result.fileSizeByte << ",";
		fsCsvOutput << paramLst[i].result.fileSizeMB << ",";
		
		startTime = convertTime2Str(paramLst[i].read_result.startTime);
		endTime = convertTime2Str(paramLst[i].read_result.endTime);
		fsCsvOutput << paramLst[i].read_result.durationInSeconds << ",";
		fsCsvOutput << startTime.c_str() << ",";
		fsCsvOutput << endTime.c_str() << ",";
		fsCsvOutput << paramLst[i].read_result.fileSizeByte << ",";
		fsCsvOutput << paramLst[i].read_result.fileSizeMB;


		fsCsvOutput << "\n";

	}
	fsCsvOutput.close();

	return sCsvFile;
}
fileInfo ReadDummyFile(string sFullPathFileName, int iThreadId)
{
	fileInfo fInfo;
	fInfo.FileName = sFullPathFileName;

	FILE *pFile;

	errno_t err;
	err = fopen_s(&pFile, sFullPathFileName.c_str(), "rb");

	if (pFile != NULL)
	{

		fseek(pFile, 0, SEEK_END);   // non-portable
		fInfo.FileSize = ftell(pFile);
		rewind(pFile);


		char c;
		long count = 0;
		do {
			c = fgetc(pFile);
			//cout << c << endl; //warning: uncommented this for debugging will caused slowness for even 1MB of file
			count++;

			if (feof(pFile))
			{
				break;
			}

		} while (1 == 1);

		fclose(pFile);
		cout << "read count:" << count << endl;

	}
	else {
		printf("ERROR: NOT ABLE TO READ TEST FILE ThreadId= %d, fileName=%s\n", iThreadId, sFullPathFileName.c_str());

	}
	return fInfo;

}



fileInfo CreateDummyFile(string sFullPathFileName, long iExpectedFileSizeBytes, int iThreadId)
{
	fileInfo fInfo;
	fInfo.FileName = sFullPathFileName;

	FILE *pFile;

	errno_t err;
	err = fopen_s(&pFile, sFullPathFileName.c_str(), "wb");

	if (pFile != NULL)
	{
		//writing dummy data in
		char * temp;
		for (long i = 0; i<iExpectedFileSizeBytes; i++){
			temp = (char *)i;
			fprintf(pFile, "%c", temp);
		}

		fseek(pFile, 0, SEEK_END);   // non-portable
		fInfo.FileSize = ftell(pFile);
		fclose(pFile);
	}
	else {
		printf("ERROR: NOT ABLE TO WRITE TEST FILE ThreadId= %d, fileName=%s\n", iThreadId, sFullPathFileName.c_str());

	}
	return fInfo;

}


//test write file
DWORD WINAPI testThread(__in LPVOID lpParameter)
{
	using namespace std;
		
	
	threadTestParam* param = (threadTestParam *)lpParameter;
	string sPath = param->TargetFilePath.c_str();
	int iFileMB = param->FileMB;
	long iExpectedFileSizeBytes = 1024 * 1024 * iFileMB;


	int iThrId = GetCurrentThreadId();
	string index = to_string(iThrId);
	string sFileName = "INTEL_" + index;
	string sFullPathFileName = sPath + sFileName + ".test";
	
	printf("Thread Started ID= %d, path=%s fname=%s\n", GetCurrentThreadId(), sPath.c_str(), sFullPathFileName.c_str());
		
	
	time_t before;
	time(&before);
	
	fileInfo fInfo = CreateDummyFile(sFullPathFileName, iExpectedFileSizeBytes, GetCurrentThreadId());
	
	time_t after;
	time(&after);

	long iFileSize = fInfo.FileSize;
	double seconds = difftime(after,before);
	double iFileSizeMB = iFileSize/1000000; //bytes to Mbs
	
	printf("WRITE Thread Id= %d, fileName=%s Duration=%.f(seconds) \nsize=%ld(Bytes)  size=%.f(MB)\n\n", GetCurrentThreadId(), sFullPathFileName.c_str(), seconds, iFileSize, iFileSizeMB);
	param->result.mode = 0;
	param->result.threadId = iThrId;
	param->result.fileName = sFullPathFileName;
	param->result.durationInSeconds = seconds;
	param->result.fileSizeByte = iExpectedFileSizeBytes;
	param->result.fileSizeMB = iFileMB;
	param->result.startTime = before;
	param->result.endTime = after;

	//step 4: READ dummy file 	
	time(&before);
	fileInfo fTemp = ReadDummyFile(sFullPathFileName, iThrId);
	time(&after);
	seconds = difftime(after, before);

	printf("READ Thread Id= %d, fileName=%s Duration=%.f(seconds) \nsize=%ld(Bytes)  size=%d (MB)\n\n", iThrId, sFullPathFileName.c_str(), seconds, iExpectedFileSizeBytes, iFileMB);
	param->read_result.mode = 1;
	param->read_result.threadId = iThrId;
	param->read_result.fileName = sFullPathFileName;
	param->read_result.durationInSeconds = seconds;
	param->read_result.fileSizeByte = iExpectedFileSizeBytes;
	param->read_result.fileSizeMB = iFileMB;
	param->read_result.startTime = before;
	param->read_result.endTime = after;

	//step 5: cleanup - remove remote file after test done
	if (remove(sFullPathFileName.c_str()) != 0){
		printf("Warning: Error remove %s  file\n", sFullPathFileName.c_str());
	}

	return 0;
}



//test copy file
DWORD WINAPI testThreadCopyFile(__in LPVOID lpParameter)
{

	using namespace std;

	
	threadTestParam* param = (threadTestParam *)lpParameter;
	
	string sPath = param->TargetFilePath.c_str();				
	int iThrId = GetCurrentThreadId();
	string index = to_string(iThrId);
	string sFileName = "INTEL_" + index;
	string sFullPathFileName = sPath + sFileName + ".test";

	int iFileMB = param->FileMB;
	long iExpectedFileSizeBytes = 1024 * 1024 * iFileMB;
	string sDummyLocalFileName = GetCurrentPath() + "\\" +sFileName;

	printf("Thread Started ID= %d, path=%s fname=%s\n\n", GetCurrentThreadId(), sPath.c_str(), sFullPathFileName.c_str());
		
	//step 1: create dummy file on local for copy, this is not suppose to count into performance
	fileInfo fDummyLocalFileInfo = CreateDummyFile(sDummyLocalFileName, iExpectedFileSizeBytes, 0);
	

	//step 2: copying dummy file from local to target
	time_t before;
	time_t after;
	double seconds;
	time(&before);
	
	//important: project need set to multi bytes unicode setting, else  will be fail uisng copyfile		
	BOOL b = CopyFile(fDummyLocalFileInfo.FileName.c_str(), sFullPathFileName.c_str(), 0);
	if (b == FALSE){
		DWORD err = GetLastError();
		std::cout << "ERR: Could not copy file to destination Thread ID =" << iThrId << " EER=" <<err <<"\n";
		
	}
	time(&after);
	seconds = difftime(after, before);

	printf("COPY Thread Id= %d, fileName=%s Duration=%.f(seconds) \nsize=%ld(Bytes)  size=%d (MB)\n\n", iThrId, sFullPathFileName.c_str(), seconds, iExpectedFileSizeBytes, iFileMB);
	param->result.mode = 1;
	param->result.threadId = iThrId;
	param->result.fileName = sFullPathFileName;
	param->result.durationInSeconds = seconds;
	param->result.fileSizeByte = iExpectedFileSizeBytes;
	param->result.fileSizeMB = iFileMB;
	param->result.startTime = before;
	param->result.endTime = after;

	//step 3: cleanup - remove dummy local file
	if (remove(sDummyLocalFileName.c_str()) != 0){
		perror ("Warning: Error remove local dummy file\n");
	}

	//step 4: READ dummy file 	
	time(&before);
	fileInfo fTemp = ReadDummyFile(sFullPathFileName, iThrId);
	time(&after);
	seconds = difftime(after, before);

	printf("READ Thread Id= %d, fileName=%s Duration=%.f(seconds) \nsize=%ld(Bytes)  size=%d (MB)\n\n", iThrId, sFullPathFileName.c_str(), seconds, iExpectedFileSizeBytes, iFileMB);
	param->read_result.mode = 1;
	param->read_result.threadId = iThrId;
	param->read_result.fileName = sFullPathFileName;
	param->read_result.durationInSeconds = seconds;
	param->read_result.fileSizeByte = iExpectedFileSizeBytes;
	param->read_result.fileSizeMB = iFileMB;
	param->read_result.startTime = before;
	param->read_result.endTime = after;

	//step 5: cleanup - remove remote file after test done
	if (remove(sFullPathFileName.c_str()) != 0){
		printf("Warning: Error remove %s  file\n", sFullPathFileName.c_str());
	}


	return 0;
}







/*

Example usage:
NetworkPerformanceTest.exe -mode 0 -user 11 -mb 10 -target \\samba.png.intel.com\nfs\site\disks\fm8_cddvs_1\DVS_Workspace\Debug\

*/
int main(int argc, char* argv[])
{
	
	
	using namespace std;
	
	/*time_t before;
	time_t after;
	
	
	string sTest = "C:\\temp\\test01.dat";
	string sTest = "\\\\samba.png.intel.com\\nfs\\site\\disks\\fm8_cddvs_1\\DVS_Workspace\\Debug\\MAHTEST.test";
		
	fileInfo fTemp01 = CreateDummyFile(sTest,100, 0);
	
	
	double seconds;
	time(&before);
	fileInfo fTemp = ReadDummyFile(sTest, 0);
	time(&after);
	seconds = difftime(after, before);

	cout << fTemp.FileName << endl;
	cout << fTemp.FileSize << endl;
	cout << "duration:"<<seconds << endl;
	
	
	getchar();
	return 0;*/

	CArgParser cmd_line(argc, argv, true);
		
	int iMode = 1;
	int iTtlThread = 0;	
	string sCSVOutputfilename;
	threadTestParam param;
	
	if (cmd_line.get_arg("-target") != "")
		param.TargetFilePath = cmd_line.get_arg("-target");
	else{		
		cout << "ERR: target not specific! e.g. " << cmd_line.get_arg(0)<<" -target //testfolder/ -user 3 -mb 2 -mode 1";
		return -1;
	}

	iMode = 1;
	if (cmd_line.get_arg("-mode") != ""){
		iMode = atoi(cmd_line.get_arg("-mode").c_str());
	}


	iTtlThread = 5;
	if (cmd_line.get_arg("-user") != ""){
		iTtlThread = atoi(cmd_line.get_arg("-user").c_str());
	}
	
	sCSVOutputfilename = "NetworkPerformanceTest_Output.csv";
	if (cmd_line.get_arg("-csv") != ""){
		sCSVOutputfilename = cmd_line.get_arg("-csv").c_str();
	}


	param.FileMB = 2; //default 2mb
	if (cmd_line.get_arg("-mb") != ""){
		param.FileMB = atoi(cmd_line.get_arg("-mb").c_str());
	}
	
	
	//sanity gate checking to avoid misuse that might caused network bandwidth congestion	
	if (iMode > 1){
		cout << "Warning: Mode (" << iTtlThread << ") unknown, reset back to 1." << endl;
		iMode = 1;
	}

	
	if (iTtlThread > 100){
		cout << "Warning: Simulating User Number (" << iTtlThread<<") has over max allowed, reset back to 100." << endl;
		iTtlThread = 100;
	}

	if (param.FileMB > 1000){
		cout << "Warning: Simulating File Size (" << iTtlThread << " MBs) has over max allowed, reset back to 2 MBs." << endl;
		param.FileMB = 1000;
	}
	
	if (param.FileMB * iTtlThread > 10000){
		cout << "Warning: Simulating File Size (" << iTtlThread << " MBs) and User (" << param.FileMB << ") combine over max allowed of total 10 GBs throughput, reset back user to 10 and file size to 1000 MBs." << endl;
		iTtlThread = 10;
		param.FileMB = 1000;
	}

	
	
	
	
	//param.TargetFilePath = "\\\\samba.png.intel.com\\nfs\\site\\disks\\fm8_cddvs_1\\DVS_Workspace\\Debug\\"; 
	//param.TargetFilePath = "\\\\pg-yockgenm-620\\works\\testdata\\"; //string sPath="C:\\testtools\\";
	//param.FileMB = 2
	//iTtlThread = 100;
		
	HANDLE* hdlThread = new HANDLE[iTtlThread];
	DWORD* thrId = new DWORD[iTtlThread];	
	vector<threadTestParam> arrParam;
		
	
	for (int i = 0; i < iTtlThread; i++){
		//copy master param to every thread, and param will hold return result in each thread
		arrParam.push_back(param);
	}
		

	for (int i = 0; i < iTtlThread; i++){
		if (iMode == 1){

			//hdlThread[i] = CreateThread(0, 0, testThreadCopyFile, (void *)&paramLst[i], 0, &thrId[i]);
			hdlThread[i] = CreateThread(0, 0, testThreadCopyFile, (void *)&arrParam[i], 0, &thrId[i]);
		}
		else{
		
			hdlThread[i] = CreateThread(0, 0, testThread, (void *)&arrParam[i], 0, &thrId[i]);
			//hdlThread[i] = CreateThread(0, 0, testThread, (void *)&paramLst[i], 0, &thrId[i]);
			//hdlThread[i] = CreateThread(0, 0, testThread, (void *)&param, 0, &thrId[i]);
		}
	}
	
	for (int i = 0; i < iTtlThread; i++){
		WaitForSingleObject(hdlThread[i], INFINITE);
		CloseHandle(hdlThread[i]);
	}
	
	//output result
	for (int i = 0; i < iTtlThread; i++){
		
		string startTime = convertTime2Str(arrParam.at(i).result.startTime);
		string endTime = convertTime2Str(arrParam.at(i).result.endTime);
		printf("OUTPUT WRITE Thread Id= %d fileName=%s\nDuration=%.f(seconds) Start=%s End=%s\nsize=%ld(Bytes) size=%d (MB)\n",
			arrParam.at(i).result.threadId, arrParam.at(i).result.fileName.c_str()
			, arrParam.at(i).result.durationInSeconds, startTime.c_str(), endTime.c_str()
			, arrParam.at(i).result.fileSizeByte, arrParam.at(i).result.fileSizeMB);

		startTime = convertTime2Str(arrParam.at(i).read_result.startTime);
		endTime = convertTime2Str(arrParam.at(i).read_result.endTime);
		printf("OUTPUT READ Thread Id= %d fileName=%s\nDuration=%.f(seconds) Start=%s End=%s\nsize=%ld(Bytes) size=%d (MB)\n\n",
			arrParam.at(i).read_result.threadId, arrParam.at(i).read_result.fileName.c_str()
			, arrParam.at(i).read_result.durationInSeconds, startTime.c_str(), endTime.c_str()
			, arrParam.at(i).read_result.fileSizeByte, arrParam.at(i).read_result.fileSizeMB);
	}

	
	string sFullPathFileName=OutputCsv(arrParam, sCSVOutputfilename);
	
	cout << "Test Result has been stored under " << sFullPathFileName.c_str() << "..." << endl;
	cout << "Testing has been complete...please press any key to exit...";	
	//getchar();
	
	return 0;
}