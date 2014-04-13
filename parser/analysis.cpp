/* This file was generated by SableCC (http://www.sablecc.org/). */

#include "node.h"
#include "token.h"
#include "list.h"
#include "prod.h"
#include "analysis.h"

rose::Analysis::~Analysis () { }

rose::AnalysisAdapter::~AnalysisAdapter () { }

void rose::AnalysisAdapter::caseStart (Start node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTEOF (TEOF node) { defaultCase(node); }
void rose::AnalysisAdapter::defaultCase (Node node) { }

void rose::AnalysisAdapter::caseTDone (TDone node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTDraw (TDraw node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTElse (TElse node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTFace (TFace node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTFork (TFork node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTMove (TMove node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTPlan (TPlan node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTProc (TProc node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTRand (TRand node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTSeed (TSeed node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTSine (TSine node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTSize (TSize node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTTemp (TTemp node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTTint (TTint node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTTurn (TTurn node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTWait (TWait node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTWhen (TWhen node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTLPar (TLPar node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTRPar (TRPar node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTPlus (TPlus node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTMinus (TMinus node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTMul (TMul node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTDiv (TDiv node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTEq (TEq node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTNe (TNe node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTLt (TLt node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTLe (TLe node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTGt (TGt node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTGe (TGe node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTAnd (TAnd node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTOr (TOr node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTAssign (TAssign node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTNeg (TNeg node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTQuestion (TQuestion node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTColon (TColon node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTNumber (TNumber node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTIdentifier (TIdentifier node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTColor (TColor node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTBlank (TBlank node) { defaultCase(node); }
void rose::AnalysisAdapter::caseTComment (TComment node) { defaultCase(node); }

void rose::AnalysisAdapter::caseAProgram (AProgram node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAWaitEvent (AWaitEvent node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAColorEvent (AColorEvent node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAProcedure (AProcedure node) { defaultCase(node); }
void rose::AnalysisAdapter::caseALocal (ALocal node) { defaultCase(node); }
void rose::AnalysisAdapter::caseADrawStatement (ADrawStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAForkStatement (AForkStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAMoveStatement (AMoveStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseASizeStatement (ASizeStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseATempStatement (ATempStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseATintStatement (ATintStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseATurnStatement (ATurnStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAFaceStatement (AFaceStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAWaitStatement (AWaitStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseASeedStatement (ASeedStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAWhenStatement (AWhenStatement node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAElseMarker (AElseMarker node) { defaultCase(node); }
void rose::AnalysisAdapter::caseANumberExpression (ANumberExpression node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAVarExpression (AVarExpression node) { defaultCase(node); }
void rose::AnalysisAdapter::caseABinaryExpression (ABinaryExpression node) { defaultCase(node); }
void rose::AnalysisAdapter::caseANegExpression (ANegExpression node) { defaultCase(node); }
void rose::AnalysisAdapter::caseASineExpression (ASineExpression node) { defaultCase(node); }
void rose::AnalysisAdapter::caseARandExpression (ARandExpression node) { defaultCase(node); }
void rose::AnalysisAdapter::caseACondExpression (ACondExpression node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAPlusBinop (APlusBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAMinusBinop (AMinusBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAMultiplyBinop (AMultiplyBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseADivideBinop (ADivideBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAEqBinop (AEqBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseANeBinop (ANeBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseALtBinop (ALtBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseALeBinop (ALeBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAGtBinop (AGtBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAGeBinop (AGeBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAAndBinop (AAndBinop node) { defaultCase(node); }
void rose::AnalysisAdapter::caseAOrBinop (AOrBinop node) { defaultCase(node); }

rose::DepthFirstAdapter::~DepthFirstAdapter () { }

void rose::DepthFirstAdapter::caseStart (Start node)
{
  inStart (node);
  node.getTEOF().apply(*this);
  node.getPProgram().apply(*this);
  outStart (node);
}

void rose::DepthFirstAdapter::defaultIn (Node node) { }
void rose::DepthFirstAdapter::defaultOut (Node node) { }

void rose::DepthFirstAdapter::inStart (Start node) { defaultIn(node); }
void rose::DepthFirstAdapter::outStart (Start node) { defaultOut(node); }

void rose::DepthFirstAdapter::inAProgram (AProgram node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAProgram (AProgram node)
{
  inAProgram (node);
  node.getEvent().apply(*this);
  node.getProcedure().apply(*this);
  outAProgram (node);
}
void rose::DepthFirstAdapter::outAProgram (AProgram node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAWaitEvent (AWaitEvent node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAWaitEvent (AWaitEvent node)
{
  inAWaitEvent (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outAWaitEvent (node);
}
void rose::DepthFirstAdapter::outAWaitEvent (AWaitEvent node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAColorEvent (AColorEvent node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAColorEvent (AColorEvent node)
{
  inAColorEvent (node);
  if ( node.getColor() ) node.getColor().apply(*this);
  outAColorEvent (node);
}
void rose::DepthFirstAdapter::outAColorEvent (AColorEvent node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAProcedure (AProcedure node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAProcedure (AProcedure node)
{
  inAProcedure (node);
  if ( node.getName() ) node.getName().apply(*this);
  node.getParams().apply(*this);
  node.getBody().apply(*this);
  outAProcedure (node);
}
void rose::DepthFirstAdapter::outAProcedure (AProcedure node) { defaultOut(node); }
void rose::DepthFirstAdapter::inALocal (ALocal node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseALocal (ALocal node)
{
  inALocal (node);
  if ( node.getName() ) node.getName().apply(*this);
  outALocal (node);
}
void rose::DepthFirstAdapter::outALocal (ALocal node) { defaultOut(node); }
void rose::DepthFirstAdapter::inADrawStatement (ADrawStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseADrawStatement (ADrawStatement node)
{
  inADrawStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  outADrawStatement (node);
}
void rose::DepthFirstAdapter::outADrawStatement (ADrawStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAForkStatement (AForkStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAForkStatement (AForkStatement node)
{
  inAForkStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getProc() ) node.getProc().apply(*this);
  node.getArgs().apply(*this);
  outAForkStatement (node);
}
void rose::DepthFirstAdapter::outAForkStatement (AForkStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAMoveStatement (AMoveStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAMoveStatement (AMoveStatement node)
{
  inAMoveStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outAMoveStatement (node);
}
void rose::DepthFirstAdapter::outAMoveStatement (AMoveStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inASizeStatement (ASizeStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseASizeStatement (ASizeStatement node)
{
  inASizeStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outASizeStatement (node);
}
void rose::DepthFirstAdapter::outASizeStatement (ASizeStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inATempStatement (ATempStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseATempStatement (ATempStatement node)
{
  inATempStatement (node);
  if ( node.getVar() ) node.getVar().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outATempStatement (node);
}
void rose::DepthFirstAdapter::outATempStatement (ATempStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inATintStatement (ATintStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseATintStatement (ATintStatement node)
{
  inATintStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outATintStatement (node);
}
void rose::DepthFirstAdapter::outATintStatement (ATintStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inATurnStatement (ATurnStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseATurnStatement (ATurnStatement node)
{
  inATurnStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outATurnStatement (node);
}
void rose::DepthFirstAdapter::outATurnStatement (ATurnStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAFaceStatement (AFaceStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAFaceStatement (AFaceStatement node)
{
  inAFaceStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outAFaceStatement (node);
}
void rose::DepthFirstAdapter::outAFaceStatement (AFaceStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAWaitStatement (AWaitStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAWaitStatement (AWaitStatement node)
{
  inAWaitStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outAWaitStatement (node);
}
void rose::DepthFirstAdapter::outAWaitStatement (AWaitStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inASeedStatement (ASeedStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseASeedStatement (ASeedStatement node)
{
  inASeedStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outASeedStatement (node);
}
void rose::DepthFirstAdapter::outASeedStatement (ASeedStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAWhenStatement (AWhenStatement node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAWhenStatement (AWhenStatement node)
{
  inAWhenStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getCond() ) node.getCond().apply(*this);
  node.getWhen().apply(*this);
  if ( node.getBetween() ) node.getBetween().apply(*this);
  node.getElse().apply(*this);
  outAWhenStatement (node);
}
void rose::DepthFirstAdapter::outAWhenStatement (AWhenStatement node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAElseMarker (AElseMarker node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAElseMarker (AElseMarker node)
{
  inAElseMarker (node);
  outAElseMarker (node);
}
void rose::DepthFirstAdapter::outAElseMarker (AElseMarker node) { defaultOut(node); }
void rose::DepthFirstAdapter::inANumberExpression (ANumberExpression node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseANumberExpression (ANumberExpression node)
{
  inANumberExpression (node);
  if ( node.getNumber() ) node.getNumber().apply(*this);
  outANumberExpression (node);
}
void rose::DepthFirstAdapter::outANumberExpression (ANumberExpression node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAVarExpression (AVarExpression node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAVarExpression (AVarExpression node)
{
  inAVarExpression (node);
  if ( node.getName() ) node.getName().apply(*this);
  outAVarExpression (node);
}
void rose::DepthFirstAdapter::outAVarExpression (AVarExpression node) { defaultOut(node); }
void rose::DepthFirstAdapter::inABinaryExpression (ABinaryExpression node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseABinaryExpression (ABinaryExpression node)
{
  inABinaryExpression (node);
  if ( node.getOp() ) node.getOp().apply(*this);
  if ( node.getLeft() ) node.getLeft().apply(*this);
  if ( node.getRight() ) node.getRight().apply(*this);
  outABinaryExpression (node);
}
void rose::DepthFirstAdapter::outABinaryExpression (ABinaryExpression node) { defaultOut(node); }
void rose::DepthFirstAdapter::inANegExpression (ANegExpression node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseANegExpression (ANegExpression node)
{
  inANegExpression (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outANegExpression (node);
}
void rose::DepthFirstAdapter::outANegExpression (ANegExpression node) { defaultOut(node); }
void rose::DepthFirstAdapter::inASineExpression (ASineExpression node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseASineExpression (ASineExpression node)
{
  inASineExpression (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outASineExpression (node);
}
void rose::DepthFirstAdapter::outASineExpression (ASineExpression node) { defaultOut(node); }
void rose::DepthFirstAdapter::inARandExpression (ARandExpression node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseARandExpression (ARandExpression node)
{
  inARandExpression (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  outARandExpression (node);
}
void rose::DepthFirstAdapter::outARandExpression (ARandExpression node) { defaultOut(node); }
void rose::DepthFirstAdapter::inACondExpression (ACondExpression node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseACondExpression (ACondExpression node)
{
  inACondExpression (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  if ( node.getCond() ) node.getCond().apply(*this);
  if ( node.getWhen() ) node.getWhen().apply(*this);
  if ( node.getElse() ) node.getElse().apply(*this);
  outACondExpression (node);
}
void rose::DepthFirstAdapter::outACondExpression (ACondExpression node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAPlusBinop (APlusBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAPlusBinop (APlusBinop node)
{
  inAPlusBinop (node);
  if ( node.getPlus() ) node.getPlus().apply(*this);
  outAPlusBinop (node);
}
void rose::DepthFirstAdapter::outAPlusBinop (APlusBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAMinusBinop (AMinusBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAMinusBinop (AMinusBinop node)
{
  inAMinusBinop (node);
  if ( node.getMinus() ) node.getMinus().apply(*this);
  outAMinusBinop (node);
}
void rose::DepthFirstAdapter::outAMinusBinop (AMinusBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAMultiplyBinop (AMultiplyBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAMultiplyBinop (AMultiplyBinop node)
{
  inAMultiplyBinop (node);
  if ( node.getMul() ) node.getMul().apply(*this);
  outAMultiplyBinop (node);
}
void rose::DepthFirstAdapter::outAMultiplyBinop (AMultiplyBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inADivideBinop (ADivideBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseADivideBinop (ADivideBinop node)
{
  inADivideBinop (node);
  if ( node.getDiv() ) node.getDiv().apply(*this);
  outADivideBinop (node);
}
void rose::DepthFirstAdapter::outADivideBinop (ADivideBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAEqBinop (AEqBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAEqBinop (AEqBinop node)
{
  inAEqBinop (node);
  if ( node.getEq() ) node.getEq().apply(*this);
  outAEqBinop (node);
}
void rose::DepthFirstAdapter::outAEqBinop (AEqBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inANeBinop (ANeBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseANeBinop (ANeBinop node)
{
  inANeBinop (node);
  if ( node.getNe() ) node.getNe().apply(*this);
  outANeBinop (node);
}
void rose::DepthFirstAdapter::outANeBinop (ANeBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inALtBinop (ALtBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseALtBinop (ALtBinop node)
{
  inALtBinop (node);
  if ( node.getLt() ) node.getLt().apply(*this);
  outALtBinop (node);
}
void rose::DepthFirstAdapter::outALtBinop (ALtBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inALeBinop (ALeBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseALeBinop (ALeBinop node)
{
  inALeBinop (node);
  if ( node.getLe() ) node.getLe().apply(*this);
  outALeBinop (node);
}
void rose::DepthFirstAdapter::outALeBinop (ALeBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAGtBinop (AGtBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAGtBinop (AGtBinop node)
{
  inAGtBinop (node);
  if ( node.getGt() ) node.getGt().apply(*this);
  outAGtBinop (node);
}
void rose::DepthFirstAdapter::outAGtBinop (AGtBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAGeBinop (AGeBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAGeBinop (AGeBinop node)
{
  inAGeBinop (node);
  if ( node.getGe() ) node.getGe().apply(*this);
  outAGeBinop (node);
}
void rose::DepthFirstAdapter::outAGeBinop (AGeBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAAndBinop (AAndBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAAndBinop (AAndBinop node)
{
  inAAndBinop (node);
  if ( node.getAnd() ) node.getAnd().apply(*this);
  outAAndBinop (node);
}
void rose::DepthFirstAdapter::outAAndBinop (AAndBinop node) { defaultOut(node); }
void rose::DepthFirstAdapter::inAOrBinop (AOrBinop node) { defaultIn(node); }
void rose::DepthFirstAdapter::caseAOrBinop (AOrBinop node)
{
  inAOrBinop (node);
  if ( node.getOr() ) node.getOr().apply(*this);
  outAOrBinop (node);
}
void rose::DepthFirstAdapter::outAOrBinop (AOrBinop node) { defaultOut(node); }

rose::ReversedDepthFirstAdapter::~ReversedDepthFirstAdapter () { }

void rose::ReversedDepthFirstAdapter::caseStart (Start node)
{
  inStart (node);
  node.getTEOF().apply(*this);
  node.getPProgram().apply(*this);
  outStart (node);
}

void rose::ReversedDepthFirstAdapter::defaultIn (Node node) { }
void rose::ReversedDepthFirstAdapter::defaultOut (Node node) { }

void rose::ReversedDepthFirstAdapter::inStart (Start node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::outStart (Start node) { defaultOut(node); }

void rose::ReversedDepthFirstAdapter::inAProgram (AProgram node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAProgram (AProgram node)
{
  inAProgram (node);
  node.getProcedure().reverse_apply(*this);
  node.getEvent().reverse_apply(*this);
  outAProgram (node);
}
void rose::ReversedDepthFirstAdapter::outAProgram (AProgram node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAWaitEvent (AWaitEvent node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAWaitEvent (AWaitEvent node)
{
  inAWaitEvent (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  outAWaitEvent (node);
}
void rose::ReversedDepthFirstAdapter::outAWaitEvent (AWaitEvent node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAColorEvent (AColorEvent node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAColorEvent (AColorEvent node)
{
  inAColorEvent (node);
  if ( node.getColor() ) node.getColor().apply(*this);
  outAColorEvent (node);
}
void rose::ReversedDepthFirstAdapter::outAColorEvent (AColorEvent node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAProcedure (AProcedure node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAProcedure (AProcedure node)
{
  inAProcedure (node);
  node.getBody().reverse_apply(*this);
  node.getParams().reverse_apply(*this);
  if ( node.getName() ) node.getName().apply(*this);
  outAProcedure (node);
}
void rose::ReversedDepthFirstAdapter::outAProcedure (AProcedure node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inALocal (ALocal node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseALocal (ALocal node)
{
  inALocal (node);
  if ( node.getName() ) node.getName().apply(*this);
  outALocal (node);
}
void rose::ReversedDepthFirstAdapter::outALocal (ALocal node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inADrawStatement (ADrawStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseADrawStatement (ADrawStatement node)
{
  inADrawStatement (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  outADrawStatement (node);
}
void rose::ReversedDepthFirstAdapter::outADrawStatement (ADrawStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAForkStatement (AForkStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAForkStatement (AForkStatement node)
{
  inAForkStatement (node);
  node.getArgs().reverse_apply(*this);
  if ( node.getProc() ) node.getProc().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outAForkStatement (node);
}
void rose::ReversedDepthFirstAdapter::outAForkStatement (AForkStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAMoveStatement (AMoveStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAMoveStatement (AMoveStatement node)
{
  inAMoveStatement (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outAMoveStatement (node);
}
void rose::ReversedDepthFirstAdapter::outAMoveStatement (AMoveStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inASizeStatement (ASizeStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseASizeStatement (ASizeStatement node)
{
  inASizeStatement (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outASizeStatement (node);
}
void rose::ReversedDepthFirstAdapter::outASizeStatement (ASizeStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inATempStatement (ATempStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseATempStatement (ATempStatement node)
{
  inATempStatement (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getVar() ) node.getVar().apply(*this);
  outATempStatement (node);
}
void rose::ReversedDepthFirstAdapter::outATempStatement (ATempStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inATintStatement (ATintStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseATintStatement (ATintStatement node)
{
  inATintStatement (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outATintStatement (node);
}
void rose::ReversedDepthFirstAdapter::outATintStatement (ATintStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inATurnStatement (ATurnStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseATurnStatement (ATurnStatement node)
{
  inATurnStatement (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outATurnStatement (node);
}
void rose::ReversedDepthFirstAdapter::outATurnStatement (ATurnStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAFaceStatement (AFaceStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAFaceStatement (AFaceStatement node)
{
  inAFaceStatement (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outAFaceStatement (node);
}
void rose::ReversedDepthFirstAdapter::outAFaceStatement (AFaceStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAWaitStatement (AWaitStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAWaitStatement (AWaitStatement node)
{
  inAWaitStatement (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outAWaitStatement (node);
}
void rose::ReversedDepthFirstAdapter::outAWaitStatement (AWaitStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inASeedStatement (ASeedStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseASeedStatement (ASeedStatement node)
{
  inASeedStatement (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outASeedStatement (node);
}
void rose::ReversedDepthFirstAdapter::outASeedStatement (ASeedStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAWhenStatement (AWhenStatement node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAWhenStatement (AWhenStatement node)
{
  inAWhenStatement (node);
  node.getElse().reverse_apply(*this);
  if ( node.getBetween() ) node.getBetween().apply(*this);
  node.getWhen().reverse_apply(*this);
  if ( node.getCond() ) node.getCond().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outAWhenStatement (node);
}
void rose::ReversedDepthFirstAdapter::outAWhenStatement (AWhenStatement node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAElseMarker (AElseMarker node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAElseMarker (AElseMarker node)
{
  inAElseMarker (node);
  outAElseMarker (node);
}
void rose::ReversedDepthFirstAdapter::outAElseMarker (AElseMarker node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inANumberExpression (ANumberExpression node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseANumberExpression (ANumberExpression node)
{
  inANumberExpression (node);
  if ( node.getNumber() ) node.getNumber().apply(*this);
  outANumberExpression (node);
}
void rose::ReversedDepthFirstAdapter::outANumberExpression (ANumberExpression node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAVarExpression (AVarExpression node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAVarExpression (AVarExpression node)
{
  inAVarExpression (node);
  if ( node.getName() ) node.getName().apply(*this);
  outAVarExpression (node);
}
void rose::ReversedDepthFirstAdapter::outAVarExpression (AVarExpression node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inABinaryExpression (ABinaryExpression node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseABinaryExpression (ABinaryExpression node)
{
  inABinaryExpression (node);
  if ( node.getRight() ) node.getRight().apply(*this);
  if ( node.getLeft() ) node.getLeft().apply(*this);
  if ( node.getOp() ) node.getOp().apply(*this);
  outABinaryExpression (node);
}
void rose::ReversedDepthFirstAdapter::outABinaryExpression (ABinaryExpression node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inANegExpression (ANegExpression node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseANegExpression (ANegExpression node)
{
  inANegExpression (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outANegExpression (node);
}
void rose::ReversedDepthFirstAdapter::outANegExpression (ANegExpression node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inASineExpression (ASineExpression node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseASineExpression (ASineExpression node)
{
  inASineExpression (node);
  if ( node.getExpression() ) node.getExpression().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outASineExpression (node);
}
void rose::ReversedDepthFirstAdapter::outASineExpression (ASineExpression node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inARandExpression (ARandExpression node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseARandExpression (ARandExpression node)
{
  inARandExpression (node);
  if ( node.getToken() ) node.getToken().apply(*this);
  outARandExpression (node);
}
void rose::ReversedDepthFirstAdapter::outARandExpression (ARandExpression node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inACondExpression (ACondExpression node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseACondExpression (ACondExpression node)
{
  inACondExpression (node);
  if ( node.getElse() ) node.getElse().apply(*this);
  if ( node.getWhen() ) node.getWhen().apply(*this);
  if ( node.getCond() ) node.getCond().apply(*this);
  if ( node.getToken() ) node.getToken().apply(*this);
  outACondExpression (node);
}
void rose::ReversedDepthFirstAdapter::outACondExpression (ACondExpression node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAPlusBinop (APlusBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAPlusBinop (APlusBinop node)
{
  inAPlusBinop (node);
  if ( node.getPlus() ) node.getPlus().apply(*this);
  outAPlusBinop (node);
}
void rose::ReversedDepthFirstAdapter::outAPlusBinop (APlusBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAMinusBinop (AMinusBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAMinusBinop (AMinusBinop node)
{
  inAMinusBinop (node);
  if ( node.getMinus() ) node.getMinus().apply(*this);
  outAMinusBinop (node);
}
void rose::ReversedDepthFirstAdapter::outAMinusBinop (AMinusBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAMultiplyBinop (AMultiplyBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAMultiplyBinop (AMultiplyBinop node)
{
  inAMultiplyBinop (node);
  if ( node.getMul() ) node.getMul().apply(*this);
  outAMultiplyBinop (node);
}
void rose::ReversedDepthFirstAdapter::outAMultiplyBinop (AMultiplyBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inADivideBinop (ADivideBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseADivideBinop (ADivideBinop node)
{
  inADivideBinop (node);
  if ( node.getDiv() ) node.getDiv().apply(*this);
  outADivideBinop (node);
}
void rose::ReversedDepthFirstAdapter::outADivideBinop (ADivideBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAEqBinop (AEqBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAEqBinop (AEqBinop node)
{
  inAEqBinop (node);
  if ( node.getEq() ) node.getEq().apply(*this);
  outAEqBinop (node);
}
void rose::ReversedDepthFirstAdapter::outAEqBinop (AEqBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inANeBinop (ANeBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseANeBinop (ANeBinop node)
{
  inANeBinop (node);
  if ( node.getNe() ) node.getNe().apply(*this);
  outANeBinop (node);
}
void rose::ReversedDepthFirstAdapter::outANeBinop (ANeBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inALtBinop (ALtBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseALtBinop (ALtBinop node)
{
  inALtBinop (node);
  if ( node.getLt() ) node.getLt().apply(*this);
  outALtBinop (node);
}
void rose::ReversedDepthFirstAdapter::outALtBinop (ALtBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inALeBinop (ALeBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseALeBinop (ALeBinop node)
{
  inALeBinop (node);
  if ( node.getLe() ) node.getLe().apply(*this);
  outALeBinop (node);
}
void rose::ReversedDepthFirstAdapter::outALeBinop (ALeBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAGtBinop (AGtBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAGtBinop (AGtBinop node)
{
  inAGtBinop (node);
  if ( node.getGt() ) node.getGt().apply(*this);
  outAGtBinop (node);
}
void rose::ReversedDepthFirstAdapter::outAGtBinop (AGtBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAGeBinop (AGeBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAGeBinop (AGeBinop node)
{
  inAGeBinop (node);
  if ( node.getGe() ) node.getGe().apply(*this);
  outAGeBinop (node);
}
void rose::ReversedDepthFirstAdapter::outAGeBinop (AGeBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAAndBinop (AAndBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAAndBinop (AAndBinop node)
{
  inAAndBinop (node);
  if ( node.getAnd() ) node.getAnd().apply(*this);
  outAAndBinop (node);
}
void rose::ReversedDepthFirstAdapter::outAAndBinop (AAndBinop node) { defaultOut(node); }
void rose::ReversedDepthFirstAdapter::inAOrBinop (AOrBinop node) { defaultIn(node); }
void rose::ReversedDepthFirstAdapter::caseAOrBinop (AOrBinop node)
{
  inAOrBinop (node);
  if ( node.getOr() ) node.getOr().apply(*this);
  outAOrBinop (node);
}
void rose::ReversedDepthFirstAdapter::outAOrBinop (AOrBinop node) { defaultOut(node); }
