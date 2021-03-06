#include "CTetris.h"
#include<math.h>
Matrix** CTetris::setOfCBlockObjects;
CTetris::~CTetris(){
    for(int i=0;i<nBlockTypes;i++)
        delete [] setOfCBlockObjects[i];
    delete [] setOfCBlockObjects;
}

void CTetris::init(int *setOfBlockArrays[], int MAX_BLK_TYPES, int MAX_BLK_DEGREES){
    Tetris::init(setOfBlockArrays,MAX_BLK_TYPES,MAX_BLK_DEGREES);
    nBlockDegrees=MAX_BLK_DEGREES;
    nBlockTypes=MAX_BLK_TYPES;
    setOfCBlockObjects=new Matrix*[MAX_BLK_TYPES];
    for(int i=0;i<MAX_BLK_TYPES;i++)
    {
        setOfCBlockObjects[i]=new Matrix[MAX_BLK_DEGREES];
        for(int j=0;j<MAX_BLK_DEGREES;j++)
        {
            int len=0;
            int where=0;
            for(int k=0;k!=-1;k=setOfBlockArrays[i*MAX_BLK_DEGREES+j][where++]){
                len++;
                if(k==1) setOfBlockArrays[i*MAX_BLK_DEGREES+j][where-1]=i+1;
            }
            len--;
            len=sqrt(len);
            if(iScreenDw<=len) iScreenDw=len;
            setOfCBlockObjects[i][j]=Matrix(setOfBlockArrays[i*MAX_BLK_DEGREES+j], len, len);
        }
    }
}

TetrisState CTetris::accept(char key){
    state=Running;
    Matrix tempBlk;
    if(key>='0'&&key<='6')
    {
        idxBlockType=key-'0';
        if(justStarted==false)
            deleteFullLines();
        iCScreen=Matrix(oCScreen);
    }
    state=Tetris::accept(key);
    currCBlk=Matrix(setOfCBlockObjects[idxBlockType][idxBlockDegree]);
    tempBlk=iCScreen.clip(top,left,top+currCBlk.get_dy(),left+currCBlk.get_dx());
    tempBlk=tempBlk.add(&currCBlk);
    oCScreen=Matrix(iCScreen);
    oCScreen.paste(&tempBlk,top,left);
    return state;
}
void CTetris::deleteFullLines(){
    for(int y=0;y<iScreenDy;y++) //반복문통해 iScreen의 모든 줄을 돌며 채워진 줄이 있는지 확인
    {
        iScreen=Matrix(oScreen);
        iCScreen=Matrix(oCScreen);
        int top=y;
        int left=iScreenDw;
        bool isDelete=false;
        Matrix tempBlk=iScreen.clip(top,left,top+1,left+iScreenDx); //한줄씩 tempBlk으로 잘라서 확인
        if(tempBlk.sum()==iScreenDx) //한줄에 있는 수들의 합이 iScreenDx와 같으면 채워진거임
            isDelete=true;
        if(isDelete==true) //해당 줄이 채워졌으면
        {
            Matrix CtempBlk; 
            tempBlk=iScreen.clip(0,left,top,left+iScreenDx); 
            //iScreen의 가장 위부터 해당 줄 바로 위까지 잘라서 tempBlk에 저장
            CtempBlk=iCScreen.clip(0,left,top,left+iScreenDx);
            //iCSreen의 가장 위부터 해당 줄 바로 위까지 잘라서 CtempBlk에 저장
            oScreen=Matrix(iScreen);
            oCScreen=Matrix(iCScreen);
            oScreen.paste(&tempBlk,1,left); //oScreen의 두번째 줄이 top이 되도록 tempBlk 붙여넣기
            oCScreen.paste(&CtempBlk,1,left); //oCScreen의 두번째 줄이 top이 되도록 CtempBlk 붙여넣기
        } 
    }
    return;
}
