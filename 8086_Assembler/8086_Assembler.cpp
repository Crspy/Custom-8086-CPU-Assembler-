// 8086_Assembler.cpp : Defines the exported functions for the DLL application.
//

#include "8086_Assembler.h"

int main(int argc,char* argv[])
{
    //FILE* myfile;
    //char* mybuff;
    //long fileSize;
    //size_t result;
    //char* mytok;
    std::string line;
    CROMBlock myrom;
    long linecount = 0;
    uint32_t PC = 0;
    
    if (argc < 2) return 0;

    std::ifstream myfile;
    try
    {
        myfile.open(argv[1], std::ios::in);
    }
    catch (std::exception& e) 
    { 
        std::cout << e.what() << "\n"; 
        std::cout << "File Not Found" << "\n";
        std::cout << "Press Any Key to exit..." << '\n';
        std::cin.get();
        return 0;
    }

    //myfile.seekg(0, std::ios_base::end);
    //fileSize = myfile.tellg();
    //myfile.clear();
    //myfile.seekg(0);
    //std::cout << fileSize << "\n";
    //mybuff = (char*)malloc(fileSize * sizeof(char));

    while (std::getline(myfile, line))
    {
        eErrorType errortype;
        tInstBlock currentInst[2];        
        char* linebuff = (char*)malloc(line.capacity());
        strcpy(linebuff,line.c_str());
        char* opToken;
        bool bMovingData = false;
        tMemAddress memadd;

        linecount++;
        // check for empty lines and comment lines
        if (line.find_first_not_of(' ') == std::string::npos
            || line.find_first_not_of('\t') == std::string::npos
            || IsCommentLine(line,linebuff))   continue;

        // check if it's IN or OUT opcode
        if (COpcode::GetOpcodeDir(line) == eOpcodeDir::OUT)
        {
            opToken = strtok(linebuff, " [], \t");
            logger(opToken);
         
            if (strcmp(opToken, "mov") == 0)
            {
                errortype = COpcode::ProcessMoveOUT(&memadd, currentInst, linebuff, &bMovingData, &myrom);
                if (errortype != eErrorType::NO_ERROR)
                {
                    CErrorHandler::PrintErrorMessage(errortype,linecount);
                    return 0;
                }

            }
            else if (strcmp(opToken, "imov") == 0)
            {
                errortype = COpcode::ProcessIndirectMoveOUT(&memadd, currentInst, linebuff);
                if (errortype != eErrorType::NO_ERROR)
                {
                    CErrorHandler::PrintErrorMessage(errortype, linecount);
                    return 0;
                }
            }
            else
            {
                CErrorHandler::PrintErrorMessage(eErrorType::UNKNOWN_OPCODE, linecount);
                return 0;
            }

        }
        else
        {
            opToken = strtok(linebuff, " ,[]/");
            logger(opToken);

            if (strcmp(opToken, "mov") == 0)
            {
                errortype = COpcode::ProcessMoveIN(&memadd, currentInst, linebuff);
                if (errortype != eErrorType::NO_ERROR)
                {
                    CErrorHandler::PrintErrorMessage(errortype, linecount);
                    return 0;
                }
            }
            else if (strcmp(opToken, "imov") == 0)
            {              
                errortype = COpcode::ProcessIndirectMoveIN(&memadd, currentInst, linebuff);
                if (errortype != eErrorType::NO_ERROR)
                {
                    CErrorHandler::PrintErrorMessage(errortype, linecount);
                    return 0;
                }

            }
            else
            {
                CErrorHandler::PrintErrorMessage(eErrorType::UNKNOWN_OPCODE, linecount);
                return 0;
            }

        }
        //printf("PC COUNT : %d\n", PC);
        //std::cout << memadd.m_Address << '\n';
        std::cout << "NeedsLoading : " << (memadd.m_bNeedLoading ? "true" : "false") << '\n';
        //printf("%d\n", reg);


        if (memadd.m_bNeedLoading)
        {
            CROMBlock::SetRomInst((tInstBlock*)&myrom.Inst[PC], &currentInst[0]);
            CROMBlock::SetRomInst((tInstBlock*)&myrom.Inst[PC+1], &currentInst[1]);
            printf("op: %d , dir_flag: %d, regID: %d, Address%d\n", currentInst[0].opcode, currentInst[0].dir_flag, currentInst[0].reg_id, currentInst[0].address);
            printf("%d , %d, %d, %d\n", currentInst[1].opcode, currentInst[1].dir_flag, currentInst[1].reg_id, currentInst[1].address);
            PC += 2;
        }
        else if (!bMovingData)
        {
            CROMBlock::SetRomInst((tInstBlock*)&myrom.Inst[PC], &currentInst[0]);
            printf("%d , %d, %d, %d\n", currentInst[0].opcode, currentInst[0].dir_flag, currentInst[0].reg_id, currentInst[0].address);
            PC++;
        }

        if (PC > (32767)) // 32768 - 1  
        {
            CErrorHandler::PrintErrorMessage(eErrorType::ROM_INSTRUCTION_SEGMENT_OVERFLOW,linecount);
            return 0;
        }
        

    }

    myfile.close();
    printf("linecount : %d\n", linecount);
    printf("PC COUNT : %d\n", PC);
    
    CROMBlockHigh highrom;
    CROMBlockLow lowrom;

    for (int i = 0; i < 65536; i++)
    {
        highrom.RomSeg[i].RomHighByte = myrom.RomSeg[i].RomSegHigh;
        lowrom.RomSeg[i].RomLowByte = myrom.RomSeg[i].RomSegLow;
    }

    
    
    std::ofstream highromfile;
    highromfile.open("high.bin", std::ios::binary | std::ios::out);
    highromfile.write((char*)&highrom, sizeof(highrom));
    highromfile.close();

    std::ofstream lowromfile;
    lowromfile.open("low.bin", std::ios::binary | std::ios::out);    
    lowromfile.write((char*)&lowrom, sizeof(lowrom));        
    lowromfile.close();
    
    myfile.close();

    std::cout << "Build Done !" << '\n';
    std::cout << "Press Any Key to exit..." << '\n';
    std::cin.get();

    return 0;
}

