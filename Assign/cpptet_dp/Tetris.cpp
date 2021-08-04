#include "Tetris.h"
// #include "Matrix.h"
#include<cmath>

int Tetris::iScreenDw=0;
Matrix** Tetris::setOfBlockObjects;
int Tetris::nBlockTypes=0;
int Tetris::nBlockDegrees=0;

Tetris::Tetris(){
    iScreenDx=0;
    iScreenDy=0;
    arrayScreen=new int[(iScreenDx+iScreenDw*2)*(iScreenDy+iScreenDw)];
    createArrayScreen(arrayScreen);
    iScreen=Matrix(arrayScreen,iScreenDy+iScreenDw,iScreenDx+2*iScreenDw);
    oScreen=Matrix(iScreen);
    idxBlockDegree=0;
    justStarted=true;
}
Tetris::Tetris(int Dy, int Dx){
    iScreenDy=Dy;
    iScreenDx=Dx;
    arrayScreen=new int[(iScreenDx+iScreenDw*2)*(iScreenDy+iScreenDw)];
    createArrayScreen(arrayScreen);
    iScreen=Matrix(arrayScreen,iScreenDy+iScreenDw,iScreenDx+2*iScreenDw);
    oScreen=Matrix(iScreen);
    idxBlockDegree=0;
    justStarted=true;
}
Tetris::~Tetris(){
    /*for(int i=0;i<nBlockTypes;i++)
        delete [] setOfBlockObjects[i];
    delete [] setOfBlockObjects;*/
    delete [] arrayScreen;
}

void Tetris::init(int *setOfBlockArrays[], int MAX_BLK_TYPES, int MAX_BLK_DEGREES){
    nBlockDegrees=MAX_BLK_DEGREES;
    nBlockTypes=MAX_BLK_TYPES;
    setOfBlockObjects=new Matrix*[MAX_BLK_TYPES];
    for(int i=0;i<MAX_BLK_TYPES;i++)
    {
        setOfBlockObjects[i]=new Matrix[MAX_BLK_DEGREES];
        for(int j=0;j<MAX_BLK_DEGREES;j++)
        {
            int len=0;
            int where=0;
            for(int k=0;k!=-1;k=setOfBlockArrays[i*MAX_BLK_DEGREES+j][where++])
                len++;
            len--;
            len=sqrt(len);
            if(iScreenDw<=len) iScreenDw=len;
            setOfBlockObjects[i][j]=Matrix(setOfBlockArrays[i*MAX_BLK_DEGREES+j], len, len);
        }
    }
}
void Tetris::kinit(){
    for(int i=0;i<nBlockTypes;i++)
        delete [] setOfBlockObjects[i];
    delete [] setOfBlockObjects;
}
void Tetris::createArrayScreen(int* arrayScreen){
    int arrayScreenDx=iScreenDw*2+iScreenDx;
    int arrayScreenDy=iScreenDy+iScreenDw;
    
    for(int y=0;y<iScreenDy;y++)
    {
        for(int x=0;x<iScreenDw;x++)
            arrayScreen[y*arrayScreenDx+x]=1;
        for(int x=iScreenDw;x<iScreenDw+iScreenDx;x++)
            arrayScreen[y*arrayScreenDx+x]=0;
        for(int x=iScreenDw+iScreenDx;x<iScreenDw+2*iScreenDx;x++)
            arrayScreen[y*arrayScreenDx+x]=1;
    }
    for(int y=iScreenDy;y<arrayScreenDy;y++)
    {
        for(int x=0;x<arrayScreenDx;x++)
            arrayScreen[y*arrayScreenDx+x]=1;
    }
}
TetrisState Tetris::accept(Obj obj){
    state=Running;
    Matrix tempBlk;
    if(obj.key==-1){
        nodelete+=obj.delRect.get_dy();
        
        Matrix upBlk=iScreen.clip(obj.delRect.get_dy(), 4, 20, 19);

        iScreen.paste(&upBlk,0,4);
        iScreen.paste(&obj.delRect,20-obj.delRect.get_dy(),4);
        tempBlk=iScreen.clip(top,left,top+currBlk.get_dy(),left+currBlk.get_dx());
        tempBlk=tempBlk.add(&currBlk);
        while(tempBlk.anyGreaterThan(1)){
            if(top==0) break;
            top=top-1;
            tempBlk=iScreen.clip(top,left,top+currBlk.get_dy(),left+currBlk.get_dx());
            tempBlk=tempBlk.add(&currBlk);
        }
        oScreen=Matrix(iScreen);
        if(tempBlk.anyGreaterThan(1))
            state=Finished;
        else oScreen.paste(&tempBlk,top,left);
        return state;
    }
    else if(obj.key>='0'&&obj.key<='6')
    {
        if(justStarted==false){
            deleteFullLines();
            if(delLines.size()>0) state=NewBlockWDel;
        }
        iScreen=Matrix(oScreen);                    
        idxBlockDegree=0;
        idxBlockType=obj.key-'0';
        currBlk=Matrix(setOfBlockObjects[idxBlockType][idxBlockDegree]);
        top=0;
        left=iScreenDw+iScreenDx/2-currBlk.get_dx()/2;
        tempBlk=iScreen.clip(top,left,top+currBlk.get_dy(),left+currBlk.get_dx());
        tempBlk=tempBlk.add(&currBlk);
        justStarted=false;
        oScreen=Matrix(iScreen);
        if(tempBlk.anyGreaterThan(1))
            state=Finished;
        else oScreen.paste(&tempBlk,top,left);
        return state;
    }
    else if(obj.key=='q')
        std::cout<<"";//do not anything
    else if(obj.key=='a')
        left-=1;
    else if(obj.key=='d')
        left+=1;
    else if(obj.key=='y')
        top+=1;
    else if(obj.key=='w')
    {
        idxBlockDegree=(idxBlockDegree+1)%nBlockDegrees;
        currBlk=Matrix(setOfBlockObjects[idxBlockType][idxBlockDegree]);
    }
    else if(obj.key==' ')
    {
        while(!tempBlk.anyGreaterThan(1))
        {
            top+=1;
            tempBlk=iScreen.clip(top,left,top+currBlk.get_dy(),left+currBlk.get_dx());
            tempBlk=tempBlk.add(&currBlk);
            state=NewBlockWODel;
        }
    }
    else
        std::cout<<"";
    tempBlk=iScreen.clip(top,left,top+currBlk.get_dy(),left+currBlk.get_dx());
    tempBlk=tempBlk.add(&currBlk);

    if(tempBlk.anyGreaterThan(1))
    {
        if(obj.key=='a')
            left+=1;
        else if(obj.key=='d')
            left-=1;
        else if(obj.key=='y')
        {
            top-=1;
            state=NewBlockWODel;
        }
        else if(obj.key=='w')
        {
            idxBlockDegree=(idxBlockDegree-1)%nBlockDegrees;
            currBlk=setOfBlockObjects[idxBlockType][idxBlockDegree];
        }
        else if(obj.key==' ')
        {
            top-=1;
            state=NewBlockWODel;
        }
        tempBlk=iScreen.clip(top,left,top+currBlk.get_dy(),left+currBlk.get_dx());
        tempBlk=tempBlk.add(&currBlk);
    }
    oScreen=Matrix(iScreen);
    oScreen.paste(&tempBlk,top,left);
    
    return state;
}
void Tetris::deleteFullLines(){
    for(int y=0;y<iScreenDy-nodelete;y++)
    {
        iScreen=Matrix(oScreen);
        int top=y;
        int left=iScreenDw;
        bool isDelete=false;
        Matrix tempBlk=iScreen.clip(top,left,top+1,left+iScreenDx);
        if(tempBlk.sum()==iScreenDx){
            isDelete=true;
            delLines.push(tempBlk);
        }
        if(isDelete==true)
        {
            tempBlk=iScreen.clip(0,left,top,left+iScreenDx);
            oScreen=Matrix(iScreen);
            oScreen.paste(&tempBlk,1,left);
        } 
    }
    return;
}
