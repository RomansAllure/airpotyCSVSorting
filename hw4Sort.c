/*
Homework 4 - Data Extraction, Conversion, and build a CSV file Output
Angel R. Roman
Michael McAlpin - COP3502 -CS-1
Fall 2016
November 26, 2016


The program is entirely my own work and that I have neither developed 
my code together with any another person, nor copied program code
from any other person, nor permitted your code to be copied or otherwise used
by any other person, nor have I copied, modified, or otherwise used program
code that I have found in any external source, including but not limited to,
online sources
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "airPdata.h"

typedef struct lListAirPdata{
	airPdata *curAirPdataP; //Pointer to the Airport Data
	struct lListAirPdata *nextAirPdataList; // pointer to next
} lListAirPdata;

void parseLine(char *line, lListAirPdata **);
void printData(lListAirPdata *);
void exportData(lListAirPdata *);
void freeLinkedList(lListAirPdata *);
float sexag2decimal(char *degreeString);
void deleteStruct(airPdata *);
void sortByLocID(lListAirPdata *);
void sortByLatitude(lListAirPdata *);

#define BUFFER_SIZE 500

int main (int argc, char *argv[]){

	// Declare input buffer and other parameters
	FILE *fid;
	char buffer[BUFFER_SIZE];
	int count = 0;
	int i;
	char sortParam; /*sort parameter*/

	// Declare a struct array and allocate memory.
	lListAirPdata *airports = NULL;
	
	// Check for command line input and open input file.
	if(argc==3){

		fid = fopen(argv[1], "r");

		if(fid == NULL){
			printf("%s ERROR: File %s not found.\n", argv[0], argv[1]);
			return 2;
		}

		sortParam = argv[2][0];
		if (sortParam != 'a' && sortParam != 'A' && sortParam != 'n' && sortParam != 'N')
		{

			printf("%s ERROR: sortParameter invalid or not found.\n", argv[0]);

			// close the input file.
			fclose(fid);

			return 3;
		}
	}
	else{
		printf("%s ERROR: sortParameter invalid or not found.\n", argv[0]);
		return 1;
	}	
	
	// Determine length of the file.
	while(fgets(buffer, BUFFER_SIZE, fid) != NULL){
		count++;
	}
	rewind(fid);

	// Read and parse each line of the input file.
	// ignore the airportser line
	fgets(buffer, BUFFER_SIZE, fid);

	for(i = 0; i < count - 1; i++){
		fgets(buffer, BUFFER_SIZE, fid);
		
		// fgets() includes the New Line delimiter in the output string. 
		// i.e. "This is my string.\n\0"
		// We will truncate the string to drop the '\n' if it is there.
		// Note: There will be no '\n' if the line is longer than the buffer.
		if(buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer)-1] = '\0';
		
		parseLine(buffer, &airports);
	}

	// close the input file.
	fclose(fid);

	// Output the data to stdout.
	//printData(count - 1, data);

	//sort
	sortByLocID(airports);
	sortByLatitude(airports);

	// Export to cvs file
	exportData(airports);

	// Free the memory used for fields of the structs.
	freeLinkedList(airports);

	return 0;

}

/*parse a line (record) and put info into apd*/
void parseLine(char *line, lListAirPdata **airports){

	int i=0, j=0, commas=0;
	char* temp;
	lListAirPdata *newNode; //new node
	lListAirPdata *current; //current node

	airPdata *apd = (airPdata *)malloc(sizeof(airPdata));
	if(apd==NULL){
        printf("malloc failed to initialize airPdata.\n");
        exit(-1);
    }

	while(commas<15){
		while(*(line+i)!=','){
			i++;
		}
		
		// strncpy does not append a '\0' to the end of the copied sub-string, so we will
		// replace the comma with '\0'. 
		*(line+i) = '\0';

		switch (commas){
		
		case 1:   //Grab the second "field" - Location ID
			apd->LocID = (char*)malloc(sizeof(char)*(i-j+1));
			if(apd->LocID==NULL){
                printf("malloc failed to initialize airPdata.LocID.\n");
                    exit(-1);
            }
            strncpy(apd->LocID, line+j, i-j+1);			

            break;
		case 2:   //Grab the third "field" - Field Name
			apd->fieldName = (char*)malloc(sizeof(char)*(i-j+1));
			if(apd->fieldName==NULL){
				printf("malloc failed to initialize airPdata.fieldName.\n");
				exit(-1);
			}
			strncpy(apd->fieldName, line+j, i-j+1);
			break;
		case 3:   //Grab the fourth "field" - City
			apd->city = (char*)malloc(sizeof(char)*(i-j+1));
			if(apd->city==NULL){
				printf("malloc failed to initialize airPdata.city.\n");
				exit(-1);
			}
			strncpy(apd->city, line+j, i-j+1);
			break;
		
		case 8:   //Grab the ninth "field" - Latitude (sexagesimal string)
			temp = (char*)malloc(sizeof(char)*(i-j+1));
			if(temp == NULL){
				printf("malloc failed to initialize airPdata.latitude.\n");
				exit(-1);
			}
			strncpy(temp, line+j, i-j+1);
			apd->latitude = sexag2decimal(temp);
			free(temp);
			break;
		case 9:   //Grab the tenth "field" - Longitude (sexagesimal string)
			temp = (char*)malloc(sizeof(char)*(i-j+1));
			if(temp == NULL){
				printf("malloc failed to initialize airPdata.longitude.\n");
				exit(-1);
			}
			strncpy(temp, line+j, i-j+1);
			apd->longitude = sexag2decimal(temp);
			free(temp);
			break;
		
		}

		j=++i;
		commas++;
	}

	//ignore Loc ID begin with numerical digits
	//or FL or X followed by 2 digits
	if (
		(apd->LocID[0] >= '0' && apd->LocID[0] <= '9') ||
		(strlen(apd->LocID) >= 4 && apd->LocID[0] == 'F' && apd->LocID[1] == 'L' &&
			apd->LocID[2] >= '0' && apd->LocID[2] <= '9' && apd->LocID[3] >= '0' && apd->LocID[3] <= '9'
		) ||
		(strlen(apd->LocID) >= 3 && apd->LocID[0] == 'X'  &&
			apd->LocID[1] >= '0' && apd->LocID[1] <= '9' && apd->LocID[2] >= '0' && apd->LocID[2] <= '9'
		)
		)
	{
		deleteStruct(apd); 
	}else{

		//create new node
		newNode = (lListAirPdata *)malloc(sizeof(lListAirPdata));
		if(newNode==NULL){
			printf("malloc failed to initialize Node.\n");
			exit(-1);
		}
		newNode->curAirPdataP = apd;
		newNode->nextAirPdataList = NULL;

		//append to linked list
		if (*airports == NULL)
		{//add a head node
			*airports = newNode;
		}else{//add as tail node
			current = *airports;

			while (current->nextAirPdataList != NULL)
			{
				current = current->nextAirPdataList;
			}

			current->nextAirPdataList = newNode;
		}
	}
}

/*print data on console*/
void printData(lListAirPdata *airports){
	int seqNumber = 1;
	lListAirPdata *current = airports; //current node

	printf("%-12s %-12s %-42s %-34s %-15s %-16s\n", "seqNumber","Code", "Name", "City", "Latitude", "Longitude");
	printf("%-12s%-12s %-42s %-34s %-15s %-16s\n", "==========", "==========", "============", "====", "========", "=========");
	
	while(current != NULL){
		printf("%-12d%-12s %-42s %-34s %-11.4f %-12.4f\n",seqNumber, current->curAirPdataP->LocID,current->curAirPdataP->fieldName,
		  current->curAirPdataP->city, current->curAirPdataP->latitude, current->curAirPdataP->longitude);

		current = current->nextAirPdataList; //next node

		seqNumber++;
	}
}

/*
export in CSV format to OUTPUT_FILE_NAME
*/
void exportData(lListAirPdata *airports){

	int seqNumber = 1;

	lListAirPdata *current = airports; //current node
	
	//print header line
	printf("%s,%s,%s,%s,%s,%s\n", "seqNumber", "code", "name", "city", "lat", "lon");

	//print records
	while(current != NULL){
		printf("%d,%s,%s,%s,%.4f,%.4f\n",seqNumber, current->curAirPdataP->LocID,current->curAirPdataP->fieldName,
			current->curAirPdataP->city, current->curAirPdataP->latitude,current->curAirPdataP->longitude);

		current = current->nextAirPdataList; //next node

		seqNumber++;
	}
}

/*free resources*/
void deleteStruct(airPdata *apd){
	free(apd->city);
	free(apd->fieldName);
	free(apd->LocID);
}

float sexag2decimal(char *degreeString){
	/*extract the content of degree string*/
	int DD, MM, SS, MAS;
	char D; /*direction*/
	float decimalDegree; /*floating point representation of the calculated decimal degrees or 0.0*/

	/*check null*/
	if (degreeString == NULL){
		return 0.0;
	}
	/*try to parse*/
	if (sscanf(degreeString, "%d-%d-%d.%d%c", &DD, &MM, &SS, &MAS, &D) != 5){
		return 0.0;
	}
	/*validate value*/
	if (DD < 0 || DD > 180 || MM < 0 || MM > 59 || SS < 0 || SS > 59 ||
		MAS < 0 || MAS > 9999 || (D != 'S' && D != 'N' && D != 'E' && D != 'W')){
		return 0.0;
	}	

	decimalDegree = (float)DD + (float)(MM / (float)60.0) + ((float)SS/(float)(60.0 * 60.0));

	if (D == 'W' || D == 'S')
	{
		decimalDegree = -decimalDegree;
	}

	return decimalDegree;
}


//free linked list
void freeLinkedList(lListAirPdata *airports){
	lListAirPdata *temp;    
	lListAirPdata *current; //current node

	current = airports;
	//iterate the linked list
	while (current != NULL)
	{
		temp = current;
		current = current->nextAirPdataList; //next node

		deleteStruct(temp->curAirPdataP);
		free(temp);
	}
}

//Sorts the airports alphabetically by the string named Loc ID with only
//one airport per letter
void sortByLocID(lListAirPdata *airports){
	lListAirPdata *current = airports;
	lListAirPdata *next;    
	airPdata *data;

	while (current != NULL)
	{
		next = current->nextAirPdataList;
		while (next != NULL)
		{
			//compare and swap
			if (current->curAirPdataP->LocID[0] > next->curAirPdataP->LocID[0])
			{
				data = current->curAirPdataP;
				current->curAirPdataP = next->curAirPdataP;
				next->curAirPdataP = data;
			}
			next = next->nextAirPdataList; //next node
		}
		current = current->nextAirPdataList; //next node
	}
}

//sort by latitude
void sortByLatitude(lListAirPdata *airports){
	lListAirPdata *current = airports;
	lListAirPdata *next;    
	airPdata *data;

	while (current != NULL)
	{
		next = current->nextAirPdataList;
		while (next != NULL)
		{
			//compare and swap
			if (current->curAirPdataP->latitude > next->curAirPdataP->latitude)
			{
				data = current->curAirPdataP;
				current->curAirPdataP = next->curAirPdataP;
				next->curAirPdataP = data;
			}
			next = next->nextAirPdataList; //next node
		}
		current = current->nextAirPdataList; //next node
	}
}