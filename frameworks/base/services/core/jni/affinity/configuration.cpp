
#define LOG_TAG "configuration"
//#define LOG_NDEBUG 0


#include "configuration.h"
#include "Process.h"

using namespace android;


int configuration::loadXML()
{
	int ret;
	int size;
	int loop;
	int code;
	#ifdef DEBUG_AFFINITY
	ALOGE("loadXML()");
	#endif
    TiXmlDocument doc;
    if(!doc.LoadFile("/system/etc/affinity_conf.xml"))
    {
    	#ifdef DEBUG_AFFINITY
        ALOGE("LoadFile fail");
		#endif
        return BAD_VALUE;
    }

    TiXmlElement* root = doc.FirstChildElement();
    if(root == NULL)
    {   
        #ifdef DEBUG_AFFINITY
		ALOGE("Failed to load file: No root element");
	    #endif
        doc.Clear();
        return BAD_VALUE;
    }
	
    if(keycode_value_vector.size()>0)
        keycode_value_vector.clear();
    if(pid_tid_vector.size()>0)
        pid_tid_vector.clear();

	pparsedata->keycode_value_vector = keycode_value_vector;
	pparsedata->pid_tid_vector = pid_tid_vector;
    for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {	
        const char* usercode;
		const char* name;
        const char* value;
        #ifdef DEBUG_AFFINITY
		ALOGD("element block ++");
		#endif
        usercode = elem->Attribute("usercode");
        pparsedata->usercode =strtol(usercode,NULL, 16);
		#ifdef DEBUG_AFFINITY
        ALOGD("usercode = %s ,usercode =%hx",usercode,pparsedata->usercode);
        #endif
		for(TiXmlElement* iterator = elem->FirstChildElement("key");iterator != NULL;iterator = iterator->NextSiblingElement("key")){
			pkeycode_value->name = (char *)iterator->Attribute("name");
			pkeycode_value->value = (char *)iterator->Attribute("value");
			#ifdef DEBUG_AFFINITY
			ALOGD("name =%s ,value = %s",pkeycode_value->name,pkeycode_value->value);
            #endif
			keycode_value_vector.push_back(*pkeycode_value);
			
			ppid_tid->pid=Process::getPidFromProcessname(pkeycode_value->name);
            Process::getTidsFromThreadname(ppid_tid->pid,pkeycode_value->value,&pid_tid_vector,ppid_tid);
			/*ppid_tid->tid=Process::getTidFromThreadname(ppid_tid->pid,pkeycode_value->value);
            
            if(ppid_tid->pid > 0 && ppid_tid->tid >0 ){
                ALOGD("pid = %d ,tid =%d",ppid_tid->pid,ppid_tid->tid);
			    pid_tid_vector.push_back(*ppid_tid);
            }*/
            
            
        }
		size = keycode_value_vector.size();
		#ifdef DEBUG_AFFINITY
        ALOGD("keycode_value_vector.size()= %d",size);
        #endif
    }
    doc.Clear();
    return OK;
}



int main(int argc, const char *argv[])
{
	ALOGE("main");
    configuration *mconfiguration = nullptr;
        if(mconfiguration == NULL){
        mconfiguration =new configuration();
    }
	 if(mconfiguration->loadXML() == BAD_VALUE){
        return 1;
	}
	//exit();
	sp<ProcessState> proc(ProcessState::self());
	ProcessState::self()->startThreadPool();
	IPCThreadState::self()->joinThreadPool();
}
