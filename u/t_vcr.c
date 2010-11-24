/**
  *  \file t_vcr.c
  *  \brief VCR Tests
  *
  *  This is a test case I've been using for years for regression-
  *  testing PCC2's VCR. The VCRs are original, although I don't know
  *  where they really were from.
  *
  *  This hard-codes the VCRs and runs them with two different
  *  configurations. It thus serves as a nice test for a new port's
  *  floating point unit.
  */

#include <phostpdk.h>
#include <assert.h>

/* Define all combat callbacks */
void DrawInitBeams(Uns16 pShip, Uns16 pNumBeams, Uns16 pCharge)
{
    (void) pShip;
    (void) pNumBeams;
    (void) pCharge;
}

void DrawInitTubes(Uns16 pShip, Uns16 pNumTubes, Uns16 pCharge)
{
    (void) pShip;
    (void) pNumTubes;
    (void) pCharge;
}

void DrawBeamRecharge(Uns16 pShip, Uns16 pBeam, Uns16 pCharge)
{
    (void) pShip;
    (void) pBeam;
    (void) pCharge;
}

void DrawTubeRecharge(Uns16 pShip, Uns16 pTube, Uns16 pCharge)
{
    (void) pShip;
    (void) pTube;
    (void) pCharge;
}

void DrawBayRecharge(Uns16 pShip, Uns16 pBay, Uns16 pCharge)
{
    (void) pShip;
    (void) pBay;
    (void) pCharge;
}

void DrawFighterAttack(Uns16 pShip, Uns16 pFighter, Int32 pPos)
{
    (void) pShip;
    (void) pFighter;
    (void) pPos;
}

void DrawEraseFighterAttack(Uns16 pShip, Uns16 pFighter, Int32 pPos)
{
    (void) pShip;
    (void) pFighter;
    (void) pPos;
}

void DrawFighterReturn(Uns16 pShip, Uns16 pFighter, Int32 pPos)
{
    (void) pShip;
    (void) pFighter;
    (void) pPos;
}

void DrawEraseFighterReturn(Uns16 pShip, Uns16 pFighter, Int32 pPos)
{
    (void) pShip;
    (void) pFighter;
    (void) pPos;
}

void DrawShipShields(Uns16 pShip, Int32 pPos, double pShields)
{
    (void) pShip;
    (void) pPos;
    (void) pShields;
}

void DrawShipDamage(Uns16 pShip, double pDamage)
{
    (void) pShip;
    (void) pDamage;
}

void DrawShipCaptured(Uns16 pShip)
{
    (void) pShip;
}

void DrawShipDestroyed(Uns16 pShip, Int32 pPos, Boolean pIsPlanet)
{
    (void) pShip;
    (void) pPos;
    (void) pIsPlanet;
}

void DrawStalemateResult(Boolean pIsPlanet2)
{
    (void) pIsPlanet2;
}

void DrawShipCrew(Uns16 pShip, double pCrew)
{
    (void) pShip;
    (void) pCrew;
}

void DrawTorpNumber(Uns16 pShip, Uns16 pTorps)
{
    (void) pShip;
    (void) pTorps;
}

void DrawFighterNumber(Uns16 pShip, Uns16 pFighters)
{
    (void) pShip;
    (void) pFighters;
}

void DrawFighterHit(Uns16 pShip, Uns16 pFighter, Int32 pPos, Int32 pEnemyPos)
{
    (void) pShip;
    (void) pFighter;
    (void) pPos;
    (void) pEnemyPos;
}
void DrawFighterHitFighter(Int32 pPos, Uns16 pFighter1, Uns16 pFighter2)
{
    (void) pPos;
    (void) pFighter1;
    (void) pFighter2;
}

void DrawTorpHit(Uns16 pShip, Int32 pPos, Int32 pEnemyPos)
{
    (void) pShip;
    (void) pPos;
    (void) pEnemyPos;
}

void DrawTorpMiss(Uns16 pShip, Int32 pPos, Int32 pEnemyPos)
{
    (void) pShip;
    (void) pPos;
    (void) pEnemyPos;
}

void DrawBeamHitFighter(Uns16 pShip, Uns16 pBeam, Int32 pPos, Uns16 pFighter, Int32 pEnemyFighterPos)
{
    (void) pShip;
    (void) pBeam;
    (void) pPos;
    (void) pFighter;
    (void) pEnemyFighterPos;
}

void DrawBeamMissedFighter(Uns16 pShip, Uns16 pBeam, Int32 pPos, Uns16 pFighter, Int32 pEnemyFighterPos)
{
    (void) pShip;
    (void) pBeam;
    (void) pPos;
    (void) pFighter;
    (void) pEnemyFighterPos;
}

void DrawBeamHitShip(Uns16 pShip, Int32 pPos, Int32 pEnemyPos)
{
    (void) pShip;
    (void) pPos;
    (void) pEnemyPos;
}

void DrawBeamMissedShip(Uns16 pShip, Int32 pPos, Int32 pEnemyPos)
{
    (void) pShip;
    (void) pPos;
    (void) pEnemyPos;
}

void DrawEraseShip(Uns16 pShip, Int32 pPos)
{
    (void) pShip;
    (void) pPos;
}

void DrawShip(Uns16 pShip, Int32 pPos)
{
    (void) pShip;
    (void) pPos;
}

void DrawBackground(void)
{ }

void DrawBeamName(Uns16 pShip, const char *pName)
{
    (void) pShip;
    (void) pName;
}

void DrawTubeName(Uns16 pShip, const char *pName)
{
    (void) pShip;
    (void) pName;
}

void DrawShipOwner(Uns16 pShip, const char *pRaceName, Boolean pIsPlanet)
{
    (void) pShip;
    (void) pRaceName;
    (void) pIsPlanet;
}

void DrawShipName(Uns16 pShip, const char *pName)
{
    (void) pShip;
    (void) pName;
}

/* DrawDistance is used to capture times */
static Uns32 gTime;

void DrawDistance(Int32 pDist)
{
    (void) pDist;
    ++gTime;
}

/******************************* Test Cases ******************************/

static void
SetupFight1(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 751;
    l->Race        = Colonies;
    l->Name        = "Bloody Mary";
    l->BeamTech    = 6;
    l->NumBeams    = 12;
    l->TorpTech    = 0;
    l->NumTubes    = 0;
    l->NumBays     = 14;
    l->Crew        = 1300;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 150;
    l->NumTorps    = 0;

    r->IsPlanet    = False;
    r->Mass        = 681;
    r->Race        = Klingons;
    r->Name        = "Cyc-9";
    r->BeamTech    = 10;
    r->NumBeams    = 15;
    r->TorpTech    = 10;
    r->NumTubes    = 13;
    r->NumBays     = 0;
    r->Crew        = 787;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 0;
    r->NumTorps    = 85;

    SetRandomSeed(30078);
    gTime = 1;
}

static void
SetupFight2(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 537;
    l->Race        = Colonies;
    l->Name        = "Al Bundy";
    l->BeamTech    = 7;
    l->NumBeams    = 7;
    l->TorpTech    = 0;
    l->NumTubes    = 0;
    l->NumBays     = 12;
    l->Crew        = 270;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 100;
    l->NumTorps    = 0;

    r->IsPlanet    = False;
    r->Mass        = 681;
    r->Race        = Klingons;
    r->Name        = "Cyc-9";
    r->BeamTech    = 10;
    r->NumBeams    = 6;
    r->TorpTech    = 10;
    r->NumTubes    = 6;
    r->NumBays     = 0;
    r->Crew        = 698;
    r->Shields     = 0;
    r->Damage      = 61;
    r->NumFighters = 0;
    r->NumTorps    = 14;

    SetRandomSeed(-28075);
    gTime = 1;
}

static void
SetupFight3(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 537;
    l->Race        = Colonies;
    l->Name        = "Al Bundy";
    l->BeamTech    = 7;
    l->NumBeams    = 7;
    l->TorpTech    = 0;
    l->NumTubes    = 0;
    l->NumBays     = 12;
    l->Crew        = 270;
    l->Shields     = 26;
    l->Damage      = 0;
    l->NumFighters = 95;
    l->NumTorps    = 0;

    r->IsPlanet    = False;
    r->Mass        = 681;
    r->Race        = Klingons;
    r->Name        = "Cyc-11";
    r->BeamTech    = 10;
    r->NumBeams    = 15;
    r->TorpTech    = 10;
    r->NumTubes    = 13;
    r->NumBays     = 0;
    r->Crew        = 787;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 0;
    r->NumTorps    = 85;

    SetRandomSeed(-16596);
    gTime = 1;
}

static void
SetupFight4(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 681;
    l->Race        = Klingons;
    l->Name        = "Cyc-11";
    l->BeamTech    = 10;
    l->NumBeams    = 14;
    l->TorpTech    = 10;
    l->NumTubes    = 12;
    l->NumBays     = 0;
    l->Crew        = 768;
    l->Shields     = 0;
    l->Damage      = 13;
    l->NumFighters = 0;
    l->NumTorps    = 43;

    r->IsPlanet    = False;
    r->Mass        = 751;
    r->Race        = Colonies;
    r->Name        = "Charlotte de Berry";
    r->BeamTech    = 7;
    r->NumBeams    = 12;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 14;
    r->Crew        = 1300;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 240;
    r->NumTorps    = 0;

    SetRandomSeed(16696);
    gTime = 1;
}

static void
SetupFight5(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 681;
    l->Race        = Klingons;
    l->Name        = "Cyc-10";
    l->BeamTech    = 10;
    l->NumBeams    = 15;
    l->TorpTech    = 10;
    l->NumTubes    = 13;
    l->NumBays     = 0;
    l->Crew        = 787;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 0;
    l->NumTorps    = 93;

    r->IsPlanet    = False;
    r->Mass        = 751;
    r->Race        = Colonies;
    r->Name        = "Charlotte de Berry";
    r->BeamTech    = 7;
    r->NumBeams    = 8;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 10;
    r->Crew        = 1279;
    r->Shields     = 0;
    r->Damage      = 34;
    r->NumFighters = 218;
    r->NumTorps    = 0;

    SetRandomSeed(-31351);
    gTime = 1;
}

static void
SetupFight6(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 681;
    l->Race        = Klingons;
    l->Name        = "Cyc-10";
    l->BeamTech    = 10;
    l->NumBeams    = 15;
    l->TorpTech    = 10;
    l->NumTubes    = 13;
    l->NumBays     = 0;
    l->Crew        = 787;
    l->Shields     = 30;
    l->Damage      = 0;
    l->NumFighters = 0;
    l->NumTorps    = 62;

    r->IsPlanet    = False;
    r->Mass        = 261;
    r->Race        = Colonies;
    r->Name        = "Bicz na Romana";
    r->BeamTech    = 2;
    r->NumBeams    = 6;
    r->TorpTech    = 10;
    r->NumTubes    = 3;
    r->NumBays     = 0;
    r->Crew        = 334;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 0;
    r->NumTorps    = 20;

    SetRandomSeed(-31121);
    gTime = 1;
}

static void
SetupFight7(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 681;
    l->Race        = Klingons;
    l->Name        = "Cynic";
    l->BeamTech    = 10;
    l->NumBeams    = 15;
    l->TorpTech    = 10;
    l->NumTubes    = 13;
    l->NumBays     = 0;
    l->Crew        = 787;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 0;
    l->NumTorps    = 0;

    r->IsPlanet    = True;
    r->Mass        = 100;
    r->Race        = Federation;
    r->Name        = "Turing";
    r->BeamTech    = 0;
    r->NumBeams    = 0;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 0;
    r->Crew        = 0;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 0;
    r->NumTorps    = 0;

    SetRandomSeed(-9166);
    gTime = 1;
}

static void
SetupFight8(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 681;
    l->Race        = Klingons;
    l->Name        = "Cyrulic";
    l->BeamTech    = 10;
    l->NumBeams    = 15;
    l->TorpTech    = 10;
    l->NumTubes    = 13;
    l->NumBays     = 0;
    l->Crew        = 787;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 0;
    l->NumTorps    = 107;

    r->IsPlanet    = True;
    r->Mass        = 157;
    r->Race        = Empire;
    r->Name        = "Platon";
    r->BeamTech    = 5;
    r->NumBeams    = 4;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 8;
    r->Crew        = 0;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 8;
    r->NumTorps    = 0;

    SetRandomSeed(-6806);
    gTime = 1;
}

static void
SetupFight9(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 171;
    l->Race        = Orions;
    l->Name        = "Orlik-4";
    l->BeamTech    = 10;
    l->NumBeams    = 5;
    l->TorpTech    = 1;
    l->NumTubes    = 3;
    l->NumBays     = 0;
    l->Crew        = 148;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 0;
    l->NumTorps    = 0;

    r->IsPlanet    = True;
    r->Mass        = 204;
    r->Race        = Robots;
    r->Name        = "Bolzano";
    r->BeamTech    = 7;
    r->NumBeams    = 6;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 10;
    r->Crew        = 0;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 10;
    r->NumTorps    = 0;

    SetRandomSeed(30372);
    gTime = 1;
}

static void
SetupFight10(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 172;
    l->Race        = Klingons;
    l->Name        = "Wybuch-E";
    l->BeamTech    = 1;
    l->NumBeams    = 12;
    l->TorpTech    = 1;
    l->NumTubes    = 1;
    l->NumBays     = 0;
    l->Crew        = 178;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 0;
    l->NumTorps    = 0;

    r->IsPlanet    = False;
    r->Mass        = 162;
    r->Race        = Tholians;
    r->Name        = "HEART OF GOLD CLASS";
    r->BeamTech    = 3;
    r->NumBeams    = 2;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 0;
    r->Crew        = 15;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 0;
    r->NumTorps    = 0;

    SetRandomSeed(2493);
    gTime = 1;
}

static void
SetupFight11(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 751;
    l->Race        = Colonies;
    l->Name        = "Alvilda the Goth";
    l->BeamTech    = 7;
    l->NumBeams    = 12;
    l->TorpTech    = 0;
    l->NumTubes    = 0;
    l->NumBays     = 14;
    l->Crew        = 1249;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 129;
    l->NumTorps    = 0;

    r->IsPlanet    = True;
    r->Mass        = 110;
    r->Race        = Rebels;
    r->Name        = "Steenrod";
    r->BeamTech    = 2;
    r->NumBeams    = 2;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 3;
    r->Crew        = 0;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 3;
    r->NumTorps    = 0;

    SetRandomSeed(-11578);
    gTime = 1;
}

static void
SetupFight12(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 237;
    l->Race        = Orions;
    l->Name        = "REKA WINA";
    l->BeamTech    = 6;
    l->NumBeams    = 2;
    l->TorpTech    = 0;
    l->NumTubes    = 0;
    l->NumBays     = 6;
    l->Crew        = 306;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 35;
    l->NumTorps    = 0;

    r->IsPlanet    = True;
    r->Mass        = 101;
    r->Race        = Robots;
    r->Name        = "Cavalieri";
    r->BeamTech    = 1;
    r->NumBeams    = 1;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 1;
    r->Crew        = 0;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 1;
    r->NumTorps    = 0;

    SetRandomSeed(3343);
    gTime = 1;
}

static void
SetupFight13(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 171;
    l->Race        = Orions;
    l->Name        = "Grawitacja";
    l->BeamTech    = 10;
    l->NumBeams    = 5;
    l->TorpTech    = 10;
    l->NumTubes    = 3;
    l->NumBays     = 0;
    l->Crew        = 148;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 0;
    l->NumTorps    = 0;

    r->IsPlanet    = True;
    r->Mass        = 144;
    r->Race        = Tholians;
    r->Name        = "Borda";
    r->BeamTech    = 5;
    r->NumBeams    = 4;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 7;
    r->Crew        = 0;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 7;
    r->NumTorps    = 0;

    SetRandomSeed(-14682);
    gTime = 1;
}

static void
SetupFight14(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 171;
    l->Race        = Orions;
    l->Name        = "Mussolini";
    l->BeamTech    = 10;
    l->NumBeams    = 5;
    l->TorpTech    = 1;
    l->NumTubes    = 3;
    l->NumBays     = 0;
    l->Crew        = 148;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 0;
    l->NumTorps    = 0;

    r->IsPlanet    = False;
    r->Mass        = 142;
    r->Race        = Cyborg;
    r->Name        = "Bambo 479";
    r->BeamTech    = 3;
    r->NumBeams    = 2;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 0;
    r->Crew        = 111;
    r->Shields     = 84;
    r->Damage      = 16;
    r->NumFighters = 0;
    r->NumTorps    = 0;

    SetRandomSeed(-7003);
    gTime = 1;
}

static void
SetupFight15(Combat_Struct* l, Combat_Struct* r)
{
    l->IsPlanet    = False;
    l->Mass        = 681;
    l->Race        = Klingons;
    l->Name        = "Chirurg";
    l->BeamTech    = 10;
    l->NumBeams    = 15;
    l->TorpTech    = 10;
    l->NumTubes    = 13;
    l->NumBays     = 0;
    l->Crew        = 787;
    l->Shields     = 100;
    l->Damage      = 0;
    l->NumFighters = 0;
    l->NumTorps    = 53;

    r->IsPlanet    = True;
    r->Mass        = 166;
    r->Race        = Tholians;
    r->Name        = "Mengoli";
    r->BeamTech    = 6;
    r->NumBeams    = 5;
    r->TorpTech    = 0;
    r->NumTubes    = 0;
    r->NumBays     = 8;
    r->Crew        = 0;
    r->Shields     = 100;
    r->Damage      = 0;
    r->NumFighters = 8;
    r->NumTorps    = 0;

    SetRandomSeed(3722);
    gTime = 1;
}

/* Test alternative combat */
static void testAC()
{
    Combat_Struct l, r;
    Boolean result = Read_HConfig_File();
    assert(result);

    /* Fight 1 */
    SetupFight1(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_DESTROYED);
    assert(l.Shields == 0);
    assert(l.Damage == 100);
    assert(l.Crew == 1241);
    assert(l.NumFighters == 92);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_VICTOR);
    assert(r.Shields == 0);
    assert(r.Damage == 61);
    assert(r.Crew == 698);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 14);

    assert(gTime == 410);

    /* Fight 2 */
    SetupFight2(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 26);
    assert(l.Damage == 0);
    assert(l.Crew == 270);
    assert(l.NumFighters == 95);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 650);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 2);

    assert(gTime == 205);

    /* Fight 3 */
    SetupFight3(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_DESTROYED);
    assert(l.Shields == 0);
    assert(l.Damage == 100);
    assert(l.Crew == 211);
    assert(l.NumFighters == 51);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_VICTOR);
    assert(r.Shields == 0);
    assert(r.Damage == 13);
    assert(r.Crew == 768);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 43);

    assert(gTime == 300);

    /* Fight 4 */
    SetupFight4(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_DESTROYED);
    assert(l.Shields == 0);
    assert(l.Damage == 100);
    assert(l.Crew == 654);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_VICTOR);
    assert(r.Shields == 0);
    assert(r.Damage == 34);
    assert(r.Crew == 1279);
    assert(r.NumFighters == 218);
    assert(r.NumTorps == 0);

    assert(gTime == 363);

    /* Fight 5 */
    SetupFight5(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 30);
    assert(l.Damage == 0);
    assert(l.Crew == 787);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 62);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 1241);
    assert(r.NumFighters == 172);
    assert(r.NumTorps == 0);

    assert(gTime == 255);

    /* Fight 6 */
    SetupFight6(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 6);
    assert(l.Damage == 0);
    assert(l.Crew == 787);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 44);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 272);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 15);

    assert(gTime == 200);

    /* Fight 7 */
    SetupFight7(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 100);
    assert(l.Damage == 0);
    assert(l.Crew == 787);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 341);

    /* Fight 8 */
    SetupFight8(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 98);
    assert(l.Damage == 0);
    assert(l.Crew == 787);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 90);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 340);

    /* Fight 9 */
    SetupFight9(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 0);
    assert(l.Damage == 75);
    assert(l.Crew == 123);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 1555);

    /* Fight 10 */
    SetupFight10(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 0);
    assert(l.Damage == 10);
    assert(l.Crew == 168);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_CAPTURED);
    assert(r.Shields == 0);
    assert(r.Damage == 10);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 23739);

    /* Fight 11 */
    SetupFight11(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 100);
    assert(l.Damage == 0);
    assert(l.Crew == 1249);
    assert(l.NumFighters == 127);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 245);

    /* Fight 12 */
    SetupFight12(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 100);
    assert(l.Damage == 0);
    assert(l.Crew == 306);
    assert(l.NumFighters == 35);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 298);

    /* Fight 13 */
    SetupFight13(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 53);
    assert(l.Damage == 0);
    assert(l.Crew == 148);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 1201);

    /* Fight 14 */
    SetupFight14(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 99);
    assert(l.Damage == 0);
    assert(l.Crew == 148);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_CAPTURED);
    assert(r.Shields == 0);
    assert(r.Damage == 66);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 505);

    /* Fight 15 */
    SetupFight15(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 91);
    assert(l.Damage == 0);
    assert(l.Crew == 787);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 37);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 335);
}

/* Test non-alternative combat */
static void testNonAC()
{
    Combat_Struct l, r;
    FILE* fp = OpenInputFile("pconfig2.src", TEXT_MODE | GAME_DIR_ONLY);
    Boolean result = Read_SomeConfig_File(fp, "pconfig2.src", "phost", True, True, True);
    assert(result);

    /* Fight 1 */
    SetupFight1(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 0);
    assert(l.Damage == 46);
    assert(l.Crew == 1161);
    assert(l.NumFighters == 120);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 786);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 48);

    assert(gTime == 291);

    /* Fight 2 */
    SetupFight2(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 10);
    assert(l.Damage == 0);
    assert(l.Crew == 270);
    assert(l.NumFighters == 94);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 698);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 8);

    assert(gTime == 210);

    /* Fight 3 */
    SetupFight3(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_DESTROYED);
    assert(l.Shields == 0);
    assert(l.Damage == 100);
    assert(l.Crew == 40);
    assert(l.NumFighters == 46);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_VICTOR);
    assert(r.Shields == 0);
    assert(r.Damage == 9);
    assert(r.Crew == 787);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 57);

    assert(gTime == 277);

    /* Fight 4 */
    SetupFight4(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_DESTROYED);
    assert(l.Shields == 0);
    assert(l.Damage == 100);
    assert(l.Crew == 764);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 19);

    assert(r.Outcome == COMBAT_VICTOR);
    assert(r.Shields == 0);
    assert(r.Damage == 31);
    assert(r.Crew == 1206);
    assert(r.NumFighters == 226);
    assert(r.NumTorps == 0);

    assert(gTime == 259);

    /* Fight 5 */
    SetupFight5(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 0);
    assert(l.Damage == 34);
    assert(l.Crew == 787);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 62);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 1090);
    assert(r.NumFighters == 169);
    assert(r.NumTorps == 0);

    assert(gTime == 285);

    /* Fight 6 */
    SetupFight6(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 0);
    assert(l.Damage == 3);
    assert(l.Crew == 780);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 49);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 229);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 17);

    assert(gTime == 200);

    /* Fight 7 */
    SetupFight7(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 100);
    assert(l.Damage == 0);
    assert(l.Crew == 787);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 511);

    /* Fight 8 */
    SetupFight8(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 100);
    assert(l.Damage == 0);
    assert(l.Crew == 787);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 104);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 377);

    /* Fight 9 */
    SetupFight9(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_DESTROYED);
    assert(l.Shields == 0);
    assert(l.Damage == 100);
    assert(l.Crew == 111);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_VICTOR);
    assert(r.Shields == 0);
    assert(r.Damage == 96);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 858);

    /* Fight 10 */
    SetupFight10(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 0);
    assert(l.Damage == 21);
    assert(l.Crew == 164);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 15);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 2847);

    /* Fight 11 */
    SetupFight11(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 100);
    assert(l.Damage == 0);
    assert(l.Crew == 1249);
    assert(l.NumFighters == 125);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 287);

    /* Fight 12 */
    SetupFight12(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 100);
    assert(l.Damage == 0);
    assert(l.Crew == 306);
    assert(l.NumFighters == 33);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 293);

    /* Fight 13 */
    SetupFight13(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 68);
    assert(l.Damage == 0);
    assert(l.Crew == 148);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 680);

    /* Fight 14 */
    SetupFight14(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 90);
    assert(l.Damage == 0);
    assert(l.Crew == 148);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 0);

    assert(r.Outcome == COMBAT_CAPTURED);
    assert(r.Shields == 0);
    assert(r.Damage == 75);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 258);

    /* Fight 15 */
    SetupFight15(&l, &r);
    Combat(&l, &r);

    assert(l.Outcome == COMBAT_VICTOR);
    assert(l.Shields == 100);
    assert(l.Damage == 0);
    assert(l.Crew == 787);
    assert(l.NumFighters == 0);
    assert(l.NumTorps == 49);

    assert(r.Outcome == COMBAT_DESTROYED);
    assert(r.Shields == 0);
    assert(r.Damage == 100);
    assert(r.Crew == 0);
    assert(r.NumFighters == 0);
    assert(r.NumTorps == 0);

    assert(gTime == 378);
}

int main()
{
    Boolean result;

    /* Initialize. Must succeed. */
    gGameDirectory = "data";
    InitPHOSTLib();
    result = ReadGlobalData();
    assert(result);

    testAC();
    testNonAC();

    FreePHOSTLib();
    return 0;
}
