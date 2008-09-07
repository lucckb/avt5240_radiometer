/* *********************************************************** *
 * This is core module for radiation measure on SBM-20 GM tube *
 * This file is a part of Dozymeter. This file is owned by     *
 * Lucjan Bryndza. (c) 2008. All rights reserved               *
 * If you need licence contact to lucjan.bryndza@ep.com.pl     *
 * *********************************************************** */

#ifndef RADIATION_H_
#define RADIATION_H_

/*----------------------------------------------------------*/

//Current radiation mode
enum radiationMode  { 
						radiationCURRENT,	//Current radiation mode 
						radiationLAST 		//Last valid radiation
					};

/*----------------------------------------------------------*/
//Setup radiation mode
enum radiationCountMode {
							radiationCountSTD=0,			//Standard algoritm
							radiationCountSHORT=40,		//Short sample buffer
							radiationCountMEDIUM=100,   //Medium sample buffer
							radiationCountHIGH=400		//Long sample buffer
						};


/*----------------------------------------------------------*/

//Get radiation calculated in uR/h
int get_radiation(enum radiationMode mode);


/*----------------------------------------------------------*/

//Setup counter with standard russian counting alg.
void setup_radiation(enum radiationCountMode mode);

/*----------------------------------------------------------*/

//In standard alghoritm called from interrupt content
void radiation_on40s_timeout_event(void);

/*----------------------------------------------------------*/





#endif /*RADIATION_H_*/
