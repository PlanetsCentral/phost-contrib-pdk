#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if 0
#ifdef __MSDOS__
#include <dir.h>
#else
#include <sys/param.h>
#endif
#endif

#include "phostpdk.h"
#include "ptscore.h"
#include "stdtyp.h"

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif


/* Globals */
#define NAME_LENGTH 80

int          gHostId;
char         gPlayers[12][NAME_LENGTH];  /* 0 is host */
const char   gPlayerFileName[11]="names.dat";

Score_Array  gScores[11+1];
const char   *gPlayerCodes="123456789ab";
Uns16        gTeams[11+1];
Boolean      IsScoreEnabled[NUM_SCORES]={True,True,True,False,False,False,
                                         False,False,False,False,False};
Score_Def    SortKey=USED_SCORE;

static const Uns16  ScoreStartLine = 3,
                    ScoreEndLine   =17;

Boolean      AddScoreToResults = True,
             MuteOperation     = False,
             GenerateLogFile   = True,
             SortScore         = True,
             VotesMode         = False;





void LoadTeams(void)
{
    FILE    *lTeamFile;

    if ((lTeamFile=OpenInputFile("teams.dat",GAME_DIR_ONLY|NO_MISSING_ERROR)) NEQ NULL)
    {
        if (11+1 EQ fread(gTeams, sizeof(Uns16), 11+1,lTeamFile))
        {
            fclose(lTeamFile);
            return; /* successful read */
        }

        fclose(lTeamFile);
    }

    /* default */
    memset(gTeams, 0, sizeof(gTeams));
}


void WriteTeams(void)
{
    FILE    *lTeamFile;

    if ((lTeamFile=OpenOutputFile("teams.dat",GAME_DIR_ONLY)) NEQ NULL)
    {
        if (11+1 NEQ fwrite(gTeams, sizeof(Uns16), 11+1,lTeamFile))
            Error("Fatal error : Can't write teams.dat\n");

        fclose(lTeamFile);
    }

}

/* generate a new team.dat structure */

void GenerateTeams(void)
{
    char    lTmpStr[255],
            *lTmpPtr;
    int     lTeamChar;
    Uns16   lTeamNr=1,
            lCount;

    while (1)
    {
        printf("\n Do you really want to delete the current team setup (y/N) ?\n");

        if (gets(lTmpStr) EQ NULL)
            return;

        if (toupper(lTmpStr[0]) NEQ 'Y')
            return;

        memset(gTeams, 0, sizeof(gTeams));

        printf("\n\nPlease enter the Teams.\n");
        printf("You will be prompted for each team, enter the players who\n");
        printf("belong to this team (players can be one of 1 2 3 4 5 6 7 8 9 a b)\n");
        printf("Example : 15b means that the team consists of players 1,5 and 11\n\n");
        printf("Just hit newline to remove all team definitions\n");

        printf("Enter team 1 (newline for end) : ");

        lTeamNr = 1; /* need this in case we go through the loop more
                            than once */

        while ((gets(lTmpStr) NEQ NULL) AND strlen(lTmpStr) /* NEQ 0 */)
        {
            const char *p;    /* NEW */

            lTmpPtr=lTmpStr;
            while (    *lTmpPtr
                   AND (p = strchr(gPlayerCodes, tolower(*lTmpPtr))) NEQ NULL)
            {
                lTeamChar = (int) (p - gPlayerCodes) + 1;
                gTeams[lTeamChar]=lTeamNr;
                lTmpPtr++;
            }
            lTeamNr++;
            printf("Enter team %d (newline for end) : ",lTeamNr);
        }

        printf("\nYou entered : \n");
        for (lCount=1; lCount<=11; lCount++)
        {
            if (gTeams[lCount])
                sprintf(lTmpStr,"%u",gTeams[lCount]);
            else
                sprintf(lTmpStr,"no team");

            printf("player %u : team %s\n",lCount,lTmpStr);
        }
        printf("\n correct (Y/n) ?");

        if ((gets(lTmpStr) EQ NULL) OR (toupper(lTmpStr[0]) NEQ 'N'))
            return;

    }
}



void BaseScore(Uns16 pId, Uns32* pUsed, Uns32* pUnused)
{
    Uns16 lCount,
          lHull;
    Uns32 lUsedScore=0,
          lUnusedScore=0;

    for (lCount=ENGINE_TECH; lCount<=TORP_TECH; lCount++)
        lUsedScore+=50*BaseTech(pId, lCount)*(BaseTech(pId, lCount)-1)*CreditScore;

    lUsedScore +=   BaseDefense(pId)*BaseDefScore
                  + BaseFighters(pId)*FighterScore
                  + BaseBuiltScore;

    for (lCount=1; lCount<=RACEHULLS; lCount++)
        if ((lHull=TrueHull(BaseOwner(pId), lCount)) NEQ 0)
            lUnusedScore +=   BaseHulls(pId, lCount)
                            * (  (HullTritCost(lHull) + HullDurCost(lHull) + HullMolyCost(lHull)) * MineralScore
                                + HullMoneyCost(lHull)*CreditScore);

    for (lCount=1; lCount<=ENGINE_NR; lCount++)
        lUnusedScore +=   BaseEngines(pId, lCount)
                        * (  (EngTritCost(lCount) + EngDurCost(lCount) + EngMolyCost(lCount)) * MineralScore
                            + EngMoneyCost(lCount)*CreditScore);

    for (lCount=1; lCount<=TORP_NR; lCount++)
        lUnusedScore +=   BaseTubes(pId, lCount)
                        * (  (TorpTritCost(lCount) + TorpDurCost(lCount) + TorpMolyCost(lCount)) * MineralScore
                            + TorpTubeCost(lCount)*CreditScore);

    for (lCount=1; lCount<=BEAM_NR; lCount++)
        lUnusedScore +=   BaseBeams(pId, lCount)
                        * (  (BeamTritCost(lCount) + BeamDurCost(lCount) + BeamMolyCost(lCount)) * MineralScore
                            + BeamMoneyCost(lCount)*CreditScore);


    for (lCount=1; lCount<=TORP_NR; lCount++)
        lUnusedScore +=   BaseTorps(pId, lCount)
                        * ( 3* MineralScore + TorpTorpCost(lCount)*CreditScore);

    *pUsed+=lUsedScore;
    *pUnused+=lUnusedScore;
}


void PlanetScore(Uns16 pId, Uns32* pUsed, Uns32* pUnused)
{
    CargoType_Def lCargo;

    *pUsed   +=   PlanetMines(pId)*MineScore
                + PlanetFactories(pId)*FactoryScore
                + PlanetDefense(pId)*DefenseScore
                + ( PlanetBuildBase(pId) ? BaseBuiltScore : 0);

    for (lCargo=TRITANIUM; lCargo<=MOLYBDENUM; lCargo++)
        *pUnused += MineralScore * PlanetCargo(pId, lCargo);

    *pUnused +=   FuelScore   * PlanetCargo(pId, NEUTRONIUM)
                + SupplyScore * PlanetCargo(pId, SUPPLIES)
                + CreditScore * PlanetCargo(pId, CREDITS);
}


void ShipScore(Uns16 pId, Uns32* pUsedFreighter, Uns32* pUsedCapital,
               Uns32* pUnused)
{
    Uns32           lUsedScore=0;
    Uns16           lHull=ShipHull(pId);
    CargoType_Def   lCargo;

    for (lCargo=TRITANIUM; lCargo<=MOLYBDENUM; lCargo++)
        *pUnused += MineralScore * ShipCargo(pId, lCargo);

    *pUnused +=   FuelScore   * ShipCargo(pId, NEUTRONIUM)
                + SupplyScore * ShipCargo(pId, SUPPLIES)
                + CreditScore * ShipCargo(pId, CREDITS);

    lUsedScore +=   (HullTritCost(lHull) + HullDurCost(lHull) + HullMolyCost(lHull)) * MineralScore
                  + HullMoneyCost(lHull)*CreditScore;

    lUsedScore +=   HullEngineNumber(lHull)
                    * (  (EngTritCost(ShipEngine(pId)) + EngDurCost(ShipEngine(pId)) + EngMolyCost(ShipEngine(pId))) * MineralScore
                        + EngMoneyCost(ShipEngine(pId))*CreditScore);

    if (ShipBeamNumber(pId) > 0)
        lUsedScore +=   ShipBeamNumber(pId)
                        * (  (BeamTritCost(ShipBeamType(pId)) + BeamDurCost(ShipBeamType(pId)) + BeamMolyCost(ShipBeamType(pId))) * MineralScore
                            + BeamMoneyCost(ShipBeamType(pId))*CreditScore);

    if (ShipTubeNumber(pId) > 0)
    {
        lUsedScore +=   ShipTubeNumber(pId)
                        * (  (TorpTritCost(ShipTorpType(pId)) + TorpDurCost(ShipTorpType(pId)) + TorpMolyCost(ShipTorpType(pId))) * MineralScore
                            + TorpTubeCost(ShipTorpType(pId))*CreditScore)
                      + ShipAmmunition(pId)
                        * ( 3* MineralScore + TorpTorpCost(ShipTorpType(pId))*CreditScore);
    }

    if (ShipBays(pId))
        lUsedScore += FighterScore*ShipAmmunition(pId);

    if (ShipBays(pId) OR ShipBeamNumber(pId) OR ShipTubeNumber(pId))
        *pUsedCapital+=lUsedScore;
    else
        *pUsedFreighter+=lUsedScore;
}

/* calculates how many 10000 mines are worth :
   The score for a certain torp is torp_score*10000/mines_per_torp */

Uns32 MineFieldMultiplier(void)
{
    Uns16   lCount;
    Uns32   lMinimum;

    lMinimum=(TorpTorpCost(1)*CreditScore + 3*MineralScore)*10000;
    for (lCount=2; lCount<=TORP_NR; lCount++)
        lMinimum=min(lMinimum,(((Uns32)TorpTorpCost(lCount))*CreditScore + 3*MineralScore)*10000/(lCount*lCount));

    return lMinimum;
}




void GenerateScore(void)
{
    Uns16        lPlayer,
                 lCount;
    RaceType_Def lOwner;
    Uns32        lMines[11+1],
                 lMineFactor=MineFieldMultiplier();

    memset(gScores,0,sizeof(Score_Array)*(11+1));
    memset(lMines,0,sizeof(Uns32)*(11+1));

    for (lCount=1; lCount<=SHIP_NR; lCount++)
        if (IsShipExist(lCount))
        {
            lOwner=ShipOwner(lCount);
            ShipScore(lCount, &gScores[lOwner][SHIP_SCORE],
            &gScores[lOwner][CAPITAL_SCORE],
            &gScores[lOwner][UNUSED_SCORE]);

            if (ShipBeamNumber(lCount) OR ShipTubeNumber(lCount) OR ShipBays(lCount))
                gScores[lOwner][SHIPS]++;
            else
                gScores[lOwner][FREIGHTERS]++;
        }

    for (lCount=1; lCount<=PLANET_NR; lCount++) {
        if (! IsPlanetExist(lCount)) continue;

        if (PlanetOwner(lCount) NEQ NoOwner)
        {
            lOwner=PlanetOwner(lCount);
            PlanetScore(lCount, &gScores[lOwner][PLANET_SCORE],
                        &gScores[lOwner][UNUSED_SCORE]);
            gScores[lOwner][PLANETS]++;

            if (IsBaseExist(lCount))
            {
                BaseScore(lCount, &gScores[lOwner][BASE_SCORE],
                          &gScores[lOwner][UNUSED_SCORE]);
                gScores[lOwner][BASES]++;
            }
        }
    }

    for (lCount=1; lCount<=GetNumMinefields(); lCount++)
        if (IsMinefieldExist(lCount))
            lMines[MinefieldOwner(lCount)]+=MinefieldUnits(lCount);
    lMines[Robots]/=4;

    for (lPlayer=Federation; lPlayer<=Colonies; lPlayer++)
    {
        gScores[lPlayer][BASE_SCORE]    *= BaseModifier;
        gScores[lPlayer][PLANET_SCORE]  *= PlanetModifier;
        gScores[lPlayer][SHIP_SCORE]    *= FreighterModifier;
        gScores[lPlayer][CAPITAL_SCORE] *= WarshipModifier;
        gScores[lPlayer][CAPITAL_SCORE]+=SpaceMineModifier*lMines[lPlayer]*lMineFactor/10000;

        for (lCount=3; lCount<NumPointScores; lCount++)
            gScores[lPlayer][USED_SCORE]+=gScores[lPlayer][lCount];
    }
}


void GenerateVotes(void)
{
    Uns16        lCount,
                 lCount2,
                 lVotes=499;
    Uns32        lTotalVotes=0;
    RaceType_Def lRace;

/* set a few switches */
    SortScore         = True;
    SortKey           = TOTAL_SCORE;
    for (lCount=1; lCount<NumScoringMethods; lCount++)
        IsScoreEnabled[lCount]=False;
    IsScoreEnabled[TOTAL_SCORE]=True;

    memset(gScores,0,sizeof(Score_Array)*(11+1));

    for (lCount=1; lCount<=PLANET_NR; lCount++) {
        if (! IsPlanetExist(lCount)) continue;

        if (PlanetOwner(lCount) NEQ NoOwner)
        {
            gScores[PlanetOwner(lCount)][TOTAL_SCORE]+=PlanetFactories(lCount)+PlanetDefense(lCount);
            if (IsBaseExist(lCount))
            {
                for (lCount2=0; lCount2<4; lCount2++)
                    gScores[PlanetOwner(lCount)][TOTAL_SCORE]+=BaseTech(lCount,lCount2)*(BaseTech(lCount,lCount2)-1)/2;
                gScores[PlanetOwner(lCount)][TOTAL_SCORE]+=BaseDefense(lCount);
            }
        }
    }

    for (lRace=Federation; lRace<=Colonies; lRace++)
        lTotalVotes+=gScores[lRace][TOTAL_SCORE];

    for (lRace=Federation; lRace<Colonies; lRace++)
    {
        lCount=gScores[lRace][TOTAL_SCORE];
        gScores[lRace][TOTAL_SCORE]=lVotes*gScores[lRace][TOTAL_SCORE]/lTotalVotes;
        lTotalVotes-=lCount;
        lVotes-=gScores[lRace][TOTAL_SCORE];
    }
    gScores[Colonies][TOTAL_SCORE]=lVotes;

    strcpy(ScoreNames[0],"votes");
}



void CondenseScores(void)
{
    Uns16 lRaceCount1, lRaceCount2, lCount;

    for (lRaceCount1=Federation; lRaceCount1<=Colonies; lRaceCount1++)
    {

/* team handling */
        if (gTeams[lRaceCount1] NEQ 0)
            for (lRaceCount2=lRaceCount1+1; lRaceCount2<=Colonies; lRaceCount2++)
	            if (gTeams[lRaceCount1] EQ gTeams[lRaceCount2])
                    for (lCount=0; lCount<NumScoringMethods; lCount++)
                    {
		                gScores[lRaceCount1][lCount]+=gScores[lRaceCount2][lCount];
                        gScores[lRaceCount2][lCount]=0;
		            }

        for (lCount=0; lCount<NumPointScores; lCount++)
            gScores[lRaceCount1][lCount]/=ScoreDivisor;

        gScores[lRaceCount1][TOTAL_SCORE]+=gScores[lRaceCount1][USED_SCORE];
        if (gScores[lRaceCount1][USED_SCORE] NEQ 0)
	    gScores[lRaceCount1][PERCENTAGE_SCORE]=1005*gScores[lRaceCount1][USED_SCORE]/(gScores[lRaceCount1][TOTAL_SCORE]*10);
    }
}


/* converts an array with the score strings to a VGAP message, returns the
   pointers to the messages */
const char** ConvertScoresToMessage(char* pLines[], char* pBuffer, Uns16 pStartColumn)
{
    Uns16    lCount,
             lMessageNr=0,
             lColumnStart,
             lColumnEnd;
    static char  *lMessageStarts[8];

/* break score into 40 column pieces */

    pBuffer[0]=0;

    if (strlen(pLines[1])>40)
    {
        lColumnEnd=40;
        while ((pLines[1][lColumnEnd-1] EQ ' ') OR (pLines[1][lColumnEnd] NEQ ' '))
           lColumnEnd--;
        lColumnStart=pStartColumn;
    }
    else
    {
        lColumnEnd=strlen(pLines[1]);
        lColumnStart=pStartColumn;
    }

    do
    {
        lMessageStarts[lMessageNr]=pBuffer;
        strcat(pBuffer,pLines[0]);
        strcat(pBuffer,"\x0D");
        for (lCount=1; lCount<ScoreEndLine; lCount++)
            if ((lCount<ScoreStartLine) OR (lCount>=ScoreStartLine+11) OR (gScores[lCount-ScoreStartLine+1][TOTAL_SCORE] NEQ 0))
            {
                if (lMessageNr EQ 0)
                    strncat(pBuffer,pLines[lCount],pStartColumn);    /* cat race name */
                else
                {
                    /* cat only race number */

                    if ((lCount>=ScoreStartLine) AND (lCount<ScoreStartLine+11))
                    {
                        strncat(pBuffer,pLines[lCount],3);
                        strcat(pBuffer," ");
                    }
                    else
                        strncat(pBuffer,pLines[lCount],4);
                }

                strncat(pBuffer,&pLines[lCount][lColumnStart],lColumnEnd-lColumnStart);
                strcat(pBuffer,"\x0D");
            }

        strcat(pBuffer,pLines[ScoreEndLine]);
        strcat(pBuffer,"\x0D");

        lColumnStart=lColumnEnd;
        lColumnEnd+=40-4;
        lColumnEnd=min(lColumnEnd,strlen(pLines[1]));

        if (lColumnEnd NEQ strlen(pLines[1]))
            while ((pLines[1][lColumnEnd-1] EQ ' ') OR (pLines[1][lColumnEnd] NEQ ' '))
                lColumnEnd--;
#if 0
        for (lCount=0; lCount<strlen(pBuffer); lCount++)
            pBuffer[lCount]+=13;
#endif
        pBuffer=lMessageStarts[lMessageNr]+strlen(pBuffer)+1;
        pBuffer[0]=0;
        lMessageNr++;
    }
    while (lColumnStart<strlen(pLines[1]));

    lMessageStarts[lMessageNr]=NULL;

    return (const char **) lMessageStarts;
}




void ReadPlayers(void)
{
	int   x;
    FILE  *PlayerFile;
	char  dummy[80];

    PlayerFile=OpenInputFile(gPlayerFileName,GAME_DIR_ONLY|TEXT_MODE|NO_MISSING_ERROR);
    if (PlayerFile EQ NULL)
    {
        Info("No names.dat file found.");
        for (x=0; x<12; x++)
            strcpy(gPlayers[x],"\n");
        return;
    }
#ifdef __TOURNAMENT__
	fgets (dummy, 78, PlayerFile);
	gHostId = atoi(dummy);

	for (x=0; x<12; x++)
#else
	for (x=1; x<12; x++)
#endif
    {
        fgets(gPlayers[x], NAME_LENGTH, PlayerFile);
        strtok(gPlayers[x], "\n");
    }

    for (x=0; x<12; x++)
        strcat(gPlayers[x],"\n");

	fclose (PlayerFile);
}


const char** GenerateScoreMessage(char* pBuffer)
{
    char          *lTmpLines[18],
                  lTmpBuffer[NAME_LENGTH],
                  lFormatString[20],
                  *lTmpPtr;
    const char    **lReturnArray;
    Uns16         lCount,
                  lCount2,
                  lShipNr,
                  lScoreType=0,
                  lNumScores=0,
                  lNumTeams=0,
                  lStartColumn,
                  lScoreLength;
    RaceType_Def  lRace;
    Boolean       IsTeamGame=False;
    FILE          *lLogFile=0;


    CondenseScores();

/* init a few structures */
    for (lCount=0; lCount<NumScoringMethods; lCount++)
        if (IsScoreEnabled[lCount])
            lNumScores++;

    for (lCount=0; lCount<=ScoreEndLine; lCount++)
    {
        lTmpLines[lCount]=(char*)MemAlloc(23+10*lNumScores+NAME_LENGTH);
        memset(lTmpLines[lCount],0,23+10*lNumScores+NAME_LENGTH);
    }

    for (lRace=Federation; lRace<=Colonies; lRace++)
        if (gTeams[lRace] NEQ 0)
            IsTeamGame=True;

/* first write the names of the races/teams */
    if (!IsTeamGame)
    {
        lScoreLength=0;
        for (lRace=Federation; lRace<=Colonies; lRace++)
            if (gScores[lRace][TOTAL_SCORE])
            {
                sprintf(lTmpLines[lRace+2],"%2u %s",lRace,RaceShortName(lRace,lTmpBuffer));
                lTmpLines[lRace+2][18]=0;
                lScoreLength=max(lScoreLength,strlen(lTmpLines[lRace+2]));
            }
        lScoreLength=min(lScoreLength,18);
        for (lRace=Federation; lRace<=Colonies; lRace++)
            if (gScores[lRace][TOTAL_SCORE])
                strncat(lTmpLines[lRace+2],"                       ", lScoreLength-strlen(lTmpLines[lRace+2]));
    }
    else
    {
        lCount=1;
        lTmpBuffer[0]=1;
        lScoreLength=0;

        while (lTmpBuffer[0])
        {
            lTmpPtr=lTmpBuffer;
            memset(lTmpBuffer,0,11);
            for (lRace=Federation; lRace<=Colonies; lRace++)
	            if (gTeams[lRace] EQ lCount)
                {
                    *lTmpPtr=((lRace<Rebels) ? '0'+lRace : 'a'+lRace-Rebels);
                    lTmpPtr++;
                }

            if (lTmpBuffer[0] NEQ 0)
            {
                lNumTeams++;
                lRace=(isdigit(lTmpBuffer[0]) ? lTmpBuffer[0]-'0' : lTmpBuffer[0]-'a'+Rebels);
                sprintf(lTmpLines[ScoreStartLine-1+lRace],"%2u Team %u : %s",lNumTeams,lNumTeams,lTmpBuffer);
                lScoreLength=max(lScoreLength,strlen(lTmpLines[lCount+2]));
            }
            lCount++;
        }
        for (lRace=Federation; lRace<=Colonies; lRace++)
            if (gScores[lRace][TOTAL_SCORE])
                if (!gTeams[lRace])
                {
                    sprintf(lTmpLines[ScoreStartLine-1+lRace],"%2u %s",lRace,RaceShortName(lRace,lTmpBuffer));
                    lScoreLength=max(lScoreLength,strlen(lTmpLines[ScoreStartLine-1+lRace]));
                }

        for (lRace=Federation; lRace<=Colonies; lRace++)
            if (gScores[lRace][TOTAL_SCORE])
                strncat(lTmpLines[lRace+2],"                       ", lScoreLength+1-strlen(lTmpLines[lRace+2]));
    }

    sprintf(lTmpBuffer,"Turn: %u                ",TurnNumber());
    strncat(lTmpLines[0],lTmpBuffer,lScoreLength);
    strncat(lTmpLines[1],"Race                  ",lScoreLength);
    strncat(lTmpLines[2],"----------------------",lScoreLength);
    strncat(lTmpLines[14],"----------------------",lScoreLength);
    sprintf(lTmpLines[15],"ptscore %s",gVersionString);

 #ifndef NO_SHIPS

    sprintf(lTmpLines[16],"\0");
    for (lCount=1, lShipNr=0; lCount<=SHIP_NR; lCount++)
        if (IsShipExist(lCount))
            lShipNr++;

    sprintf(lTmpLines[17],"ship slots : %u used, %u empty",lShipNr, SHIP_NR-lShipNr);

#endif

#ifdef __BETA_RELEASE__
    sprintf(lTmpBuffer," beta v%u",BETA_RELEASE);
    strcat(lTmpLines[15],lTmpBuffer);
#endif
    lStartColumn=lScoreLength+1;

    for (lScoreType=0; lScoreType<NumScoringMethods; lScoreType++)
        if (IsScoreEnabled[lScoreType])
        {

/* determine the number of columns we need */
            lScoreLength=0;
            for (lRace=Federation; lRace<=Colonies; lRace++)
            {
                sprintf(lTmpBuffer,"%lu",gScores[lRace][lScoreType]);
                lScoreLength=max(lScoreLength,strlen(lTmpBuffer));
            }
            lScoreLength=max(lScoreLength,strlen(ScoreNames[lScoreType]))+1;

            if (lScoreType EQ PERCENTAGE_SCORE)
               lScoreLength++;

            sprintf(lFormatString,"%c%us",'%',lScoreLength);
            sprintf(lTmpBuffer,lFormatString,ScoreNames[lScoreType]);
            strcat(lTmpLines[1],lTmpBuffer);
            strncat(lTmpLines[2],"-------------------",lScoreLength);
            strncat(lTmpLines[14],"-------------------",lScoreLength);
            sprintf(lFormatString,"%c%ulu",'%',lScoreLength);
            if (lScoreType EQ PERCENTAGE_SCORE)
                sprintf(lFormatString,"%c%ulu%c",'%',lScoreLength-1,'%');

            for (lRace=Federation; lRace<=Colonies; lRace++)
                if (gScores[lRace][TOTAL_SCORE] NEQ 0)
                {
                    sprintf(lTmpBuffer, lFormatString, gScores[lRace][lScoreType]);
                    strcat(lTmpLines[lRace+2],lTmpBuffer);
                }
        }

/* sort scores (linear sort because we have to switch 2 different structures) */
    if (SortScore)
        for (lCount=1; lCount<=10; lCount++)
            for (lCount2=lCount+1; lCount2<=11; lCount2++)
                if (gScores[lCount][SortKey]<gScores[lCount2][SortKey])
                {
                    memcpy(&gScores[0],&gScores[lCount],sizeof(Score_Array));
                    memcpy(&gScores[lCount],&gScores[lCount2],sizeof(Score_Array));
                    memcpy(&gScores[lCount2],&gScores[0],sizeof(Score_Array));
                    lTmpPtr=lTmpLines[lCount+2];
                    lTmpLines[lCount+2]=lTmpLines[lCount2+2];
                    lTmpLines[lCount2+2]=lTmpPtr;
                    strcpy(lTmpBuffer,gPlayers[lCount]);
                    strcpy(gPlayers[lCount],gPlayers[lCount2]);
                    strcpy(gPlayers[lCount2], lTmpBuffer);
                }



    if (GenerateLogFile)
        lLogFile=OpenOutputFile("ptscore.log",GAME_DIR_ONLY|TEXT_MODE);

#ifdef __TOURNAMENT__
    fprintf(lLogFile,"Tournament game %2u\n",gHostId);
    fprintf(lLogFile,"==================\n\n");
    fprintf(lLogFile,"Host: %s\n",gPlayers[0]);
    if (!MuteOperation)
    {
        printf("\nTournament game %2u\n",gHostId);
        printf("==================\n\n");
        printf("Host: %s\n",gPlayers[0]);
    }

#endif

    sprintf(lFormatString,"%c%us  %cs",'%',strlen(lTmpLines[2]),'%');
    for (lCount=0; lCount<=ScoreEndLine; lCount++)
        if (strlen(lTmpLines[lCount]))
        {
            if (!MuteOperation)
            {
                if ((lCount>=3) AND (lCount<=13))
                    printf(lFormatString,lTmpLines[lCount], gPlayers[lCount-2]);
                else
                    printf("%s\n",lTmpLines[lCount]);
            }

            if (GenerateLogFile)
            {
                if ((lCount>=3) AND (lCount<=13))
                    fprintf(lLogFile,lFormatString,lTmpLines[lCount], gPlayers[lCount-2]);
                else
                    fprintf(lLogFile,"%s\n",lTmpLines[lCount]);
            }
        }

    if (GenerateLogFile)
        fclose(lLogFile);

    lReturnArray=ConvertScoresToMessage(lTmpLines, pBuffer, lStartColumn);
    for (lCount=0; lCount<=ScoreEndLine; lCount++)
        free(lTmpLines[lCount]);

    return lReturnArray;
}

void WriteScoreMessages(void)
{
    char          *lScoreBuffer=(char*)MemAlloc(4000);
    const char    **lMessages;
    RaceType_Def  lRace;
    int           lNumMessages;

    printf("\ngenerating score ...\n");
    if (VotesMode)
        GenerateVotes();
    else
        GenerateScore();

    lMessages=GenerateScoreMessage(lScoreBuffer);

    for (lNumMessages=0; lMessages[lNumMessages] NEQ 0; lNumMessages++)
        /* NULL STATEMENT */ ;

    if (AddScoreToResults)
    {
        printf("\nadding score to results\n");
        for (lRace=Federation; lRace<=Colonies; lRace++)
            if (PlayerIsActive(lRace))
            {
                printf(".");
                WriteMessageToRST(lRace, lNumMessages, lMessages);
            }
    }
}

void PtscoreInfo(void)
{
    printf("\nPTSCORE %s",gVersionString);
#ifdef __BETA_RELEASE__
    printf(" beta release %u\n\n",BETA_RELEASE);
#else
    printf("\n\n");
#endif

    printf("Usage : ptscore [options] [gamedirectory] [rootdirectory]\n");
    printf("Control options :\n");
    printf("       -h  : this help\n");
    printf("       -r  : don't append score to results\n");
    printf("       -m  : mute operation (don't print the score to the screen)\n");
    printf("       -l  : don't generate a ptscore.log file\n");
    printf("       -t  : configure teams\n\n");
    printf("Scoring options :\n");
    printf("       -v  : votes mode\n");
    printf("       -T  : disable total score\n");
    printf("       -U  : disable used score\n");
    printf("       -R  : disable used/total relation\n");
    printf("       -P  : enable planetary detailed score     -p  : enable planets count\n");
    printf("       -B  : enable starbase detailed score      -b  : enable base count\n");
    printf("       -W  : enable warship detailed score       -w  : enable warship count\n");
    printf("       -F  : enable freighter score              -f  : enable freighter count\n");
    printf("       -S? : sort score by [t]otal, [u]sed, [r]atio, [b]ases,\n");
    printf("             [p]lanets, [w]arships, [f]reighters or [0] race number\n");

}


void ProcessOptions(int argc, char* argv[])
{
    static char lDirectory[2]=".";
    Uns16 lArgNr, lOptionNr;

    Boolean lDoGenerateTeams = False;

    gGameDirectory = gRootDirectory = NULL;

    for (lArgNr=1; lArgNr<argc; lArgNr++)
        if (argv[lArgNr][0] NEQ '-') /* no switch */
        {
            if (gGameDirectory EQ NULL)
                gGameDirectory=argv[lArgNr];
            else
                if (gRootDirectory EQ NULL)
                    gRootDirectory=argv[lArgNr];
                else
                    ErrorExit("\nCan't process argument %s\nCall 'ptscore -h' to get help\n",argv[lArgNr]);
        }
        else
            for (lOptionNr=1; lOptionNr<strlen(argv[lArgNr]); lOptionNr++)
                switch(argv[lArgNr][lOptionNr])
                {
                    case 'T' : IsScoreEnabled[TOTAL_SCORE]=False;
                               break;
                    case 'U' : IsScoreEnabled[USED_SCORE]=False;
                               break;
                    case 'R' : IsScoreEnabled[PERCENTAGE_SCORE]=False;
                               break;
                    case 'W' : IsScoreEnabled[CAPITAL_SCORE]=True;
                               break;
                    case 'F' : IsScoreEnabled[SHIP_SCORE]=True;
                               break;
                    case 'B' : IsScoreEnabled[BASE_SCORE]=True;
                               break;
                    case 'P' : IsScoreEnabled[PLANET_SCORE]=True;
                               break;

                    case 'S' : lOptionNr++;
                               switch(argv[lArgNr][lOptionNr])
                               {
                                    case 't' : SortKey=TOTAL_SCORE;
                                               break;
                                    case 'u' : SortKey=USED_SCORE;
                                               break;
                                    case 'r' : SortKey=PERCENTAGE_SCORE;
                                               break;
                                    case 'f' : SortKey=SHIP_SCORE;
                                               break;
                                    case 'b' : SortKey=BASE_SCORE;
                                               break;
                                    case 'p' : SortKey=PLANET_SCORE;
                                               break;
                                    case 'w' : SortKey=CAPITAL_SCORE;
                                               break;
                                    case '0' : SortScore=False;
                                               break;
                                    default  : printf("unknown sort key : %c\n",argv[lArgNr][lOptionNr]);
                                               break;
                               }
                               break;

                    case 'h' : PtscoreInfo();
                               FreePHOSTLib();
                               exit(0);
                    case 'r' : AddScoreToResults=False;
                               break;
                    case 'm' : MuteOperation=True;
                               break;
                    case 'v' : VotesMode=True;
                               break;
                    case 'l' : GenerateLogFile=False;
                               break;
                    case 'w' : IsScoreEnabled[SHIPS]=True;
                               break;
                    case 'f' : IsScoreEnabled[FREIGHTERS]=True;
                               break;
                    case 'b' : IsScoreEnabled[BASES]=True;
                               break;
                    case 'p' : IsScoreEnabled[PLANETS]=True;
                               break;
                    case 't' : lDoGenerateTeams = True;
                               break;
                    default  : printf("Unknown option : -%c\n",argv[lArgNr][lOptionNr]);
                               break;
                }

    if (gGameDirectory EQ NULL)
        gGameDirectory = lDirectory;
    if (gRootDirectory EQ NULL)
        gRootDirectory = lDirectory;

    if (lDoGenerateTeams) {
        GenerateTeams();
        WriteTeams();
        FreePHOSTLib();
        exit(0);
    }
}



int
main(int argc, char *argv[])
{
    if (argc>1)
        ProcessOptions(argc,argv);

    printf("\ninitializing data ...\n");
    InitPHOSTLib();
    LoadTeams();

    ReadPlayers();
    if (! ReadGlobalData()) goto done;
    if (! ReadHostData()) goto done;

    WriteScoreMessages();

done:
    FreePHOSTLib();

    return 0;
}
