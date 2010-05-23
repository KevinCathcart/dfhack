// Creature dump

#include <iostream>
#include <iomanip>
#include <sstream>
#include <climits>
#include <integers.h>
#include <vector>
using namespace std;

#include <DFGlobal.h>
#include <DFError.h>
#include <DFTypes.h>
#include <DFContextManager.h>
#include <DFContext.h>
#include <DFMemInfo.h>
#include <DFProcess.h>
#include <DFTypes.h>
#include <modules/Buildings.h>
#include <modules/Materials.h>
#include <modules/Position.h>
#include <DFMiscUtils.h>

int main (int argc,const char* argv[])
{
    int lines = 16;
    int mode = 0;
    if (argc < 2 || argc > 3)
    {
        /*
        cout << "usage:" << endl;
        cout << argv[0] << " object_name [number of lines]" << endl;
        #ifndef LINUX_BUILD
            cout << "Done. Press any key to continue" << endl;
            cin.ignore();
        #endif
        return 0;
        */
    }
    else if(argc == 3)
    {
        string s = argv[2]; //blah. I don't care
        istringstream ins; // Declare an input string stream.
        ins.str(s);        // Specify string to read.
        ins >> lines;     // Reads the integers from the string.
        mode = 1;
    }
    
    map <uint32_t, string> custom_workshop_types;
    
    DFHack::ContextManager DFMgr ("Memory.xml");
    DFHack::Context *DF;
    try
    {
        DF = DFMgr.getSingleContext();
        DF->Attach();
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        #ifndef LINUX_BUILD
            cin.ignore();
        #endif
        return 1;
    }
    
    DFHack::memory_info * mem = DF->getMemoryInfo();
    DFHack::Buildings * Bld = DF->getBuildings();
    DFHack::Position * Pos = DF->getPosition();
    
    uint32_t numBuildings;
    if(Bld->Start(numBuildings))
    {
        Bld->ReadCustomWorkshopTypes(custom_workshop_types);
        if(mode)
        {
            cout << numBuildings << endl;
            vector < uint32_t > addresses;
            for(uint32_t i = 0; i < numBuildings; i++)
            {
                DFHack::t_building temp;
                Bld->Read(i, temp);
                if(temp.type != 0xFFFFFFFF) // check if type isn't invalid
                {
                    string typestr;
                    mem->resolveClassIDToClassname(temp.type, typestr);
                    cout << typestr << endl;
                    if(typestr == argv[1])
                    {
                        //cout << buildingtypes[temp.type] << " 0x" << hex << temp.origin << endl;
                        //hexdump(DF, temp.origin, 16);
                        addresses.push_back(temp.origin);
                    }
                }
                else
                {
                    // couldn't translate type, print out the vtable
                    cout << "unknown vtable: " << temp.vtable << endl;
                }
            }
            interleave_hex(DF,addresses,lines / 4);
        }
        else // mode
        {
            int32_t x,y,z;
            Pos->getCursorCoords(x,y,z);
            if(x != -30000)
            {
                for(uint32_t i = 0; i < numBuildings; i++)
                {
                    DFHack::t_building temp;
                    Bld->Read(i, temp);
                    if(    (uint32_t)x >= temp.x1
                        && (uint32_t)x <= temp.x2
                        && (uint32_t)y >= temp.y1
                        && (uint32_t)y <= temp.y2
                        && (uint32_t)z == temp.z
                      )
                    {
                        string typestr;
                        mem->resolveClassIDToClassname(temp.type, typestr);
                        printf("Address 0x%x, type %d (%s), %d/%d/%d\n",temp.origin, temp.type, typestr.c_str(), temp.x1,temp.y1,temp.z);
                        printf("Material %d %d\n", temp.material.type, temp.material.index);
                        int32_t custom;
                        if((custom = Bld->GetCustomWorkshopType(temp)) != -1)
                        {
                            printf("Custom workshop type %s (%d)\n",custom_workshop_types[custom].c_str(),custom);
                        }
                        hexdump(DF,temp.origin,120);
                    }
                }
            }
            else
            {
                cout << numBuildings << endl;
                for(uint32_t i = 0; i < numBuildings; i++)
                {
                    DFHack::t_building temp;
                    Bld->Read(i, temp);
                    string typestr;
                    mem->resolveClassIDToClassname(temp.type, typestr);
                    printf("Address 0x%x, type %d (%s), %d/%d/%d\n",temp.origin, temp.type, typestr.c_str(), temp.x1,temp.y1,temp.z);
                }
            }
        }
        Bld->Finish();
    }
    else
    {
        cerr << "buildings not supported for this DF version" << endl;
    }

    DF->Detach();
    #ifndef LINUX_BUILD
        cout << "Done. Press any key to continue" << endl;
        cin.ignore();
    #endif
    return 0;
}
