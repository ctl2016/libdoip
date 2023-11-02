#include <iostream>
#include <unistd.h>
#include <atomic>
#include <thread>
#include <taskflow/taskflow.hpp>
#include <unordered_map>

#include <initializer_list>
#include <cstdarg>
#include <list>
#include <typeinfo>

enum TaskPrio
{
    HI = 0,
    NO = 1,
    LO = 2,
};

class TaskModule
{
public:
    TaskModule() : m_priority(TaskPrio::NO)
    {
    }

    virtual const std::string Name() const = 0;
    virtual bool IsCondition() const = 0;
    virtual uint32_t Run() = 0;

    void SetPriority(TaskPrio prio)
    {
        m_priority = prio;
    }

    TaskPrio GetPriority()
    {
        return m_priority;
    }

    TaskModule& operator >> (TaskModule* task)
    {
        //std::cout << "TaskModule: " << Name() << " >> " << task->Name() << "\n";
		m_lstNext.push_back(task);
        return *task;
    }

    TaskModule& Before(const std::initializer_list<TaskModule*>& tasks)
    {
        for (auto& t : tasks)
        {
            //std::cout << "TaskModule: " << Name() << " Before >> " << t->Name() << "\n";
            m_lstNext.push_back(t);
        }

        return *this;
    }

    TaskModule& After(const std::initializer_list<TaskModule*>& tasks)
    {
        for (auto& t : tasks)
        {
            //std::cout << "TaskModule: " << Name() << " Before >> " << t->Name() << "\n";
            m_lstPrev.push_back(t);
        }

        return *this;
    }

    std::list<TaskModule*>& GetNextList()
    {
        return m_lstNext;
    }

    std::list<TaskModule*>& GetPrevList()
    {
        return m_lstPrev;
    }

protected:
    TaskPrio m_priority;
    std::list<TaskModule*> m_lstNext;
    std::list<TaskModule*> m_lstPrev;
};

template<typename TRunner, bool bIsCondition = false>
class TModule : public TaskModule
{
public:
    const std::string Name() const override
    {
        return typeid(TRunner).name();
    }

    bool IsCondition() const override
	{
        return bIsCondition;
    }

    uint32_t Run() override
    {
        std::cout << "Run class: " << Name() << " begin\n";
        return m_Runner.Run();
    }

private:
    TRunner m_Runner;
};

class TaskExecutor
{
public:
    TaskExecutor(const std::string& strName, const std::initializer_list<TaskModule*>& tasks): m_taskflow(strName)
    {
        for (auto& t : tasks)
        {
            std::set<TaskModule*> visited;
            visit(t, visited, [=](TaskModule* mod, TaskModule* pPrev, TaskModule* pNext) {

                    tf::Task* prev = nullptr;
                    tf::Task* next = nullptr;
                    tf::Task* curr = FindTask(mod);

                    if(nullptr == curr)
                    {
                        curr = AddTask(mod);
                    }

                    if(pPrev != nullptr)
                    {
                        prev = FindTask(pPrev);

                        if(nullptr == prev)
                        {
                            prev = AddTask(pPrev);
                        }
                        
                        (*curr).succeed(*prev);
                    }

                    if(pNext != nullptr)
                    {
                        next = FindTask(pNext);

                        if(nullptr == next)
                        {
                            next = AddTask(pNext);
                        }

                        (*curr).precede(*next);
                    }

                    //std::cout << (pPrev ? pPrev->Name():"")<< " < " << pCurr->Name() << " > " << (pNext ? pNext->Name():"")<< std::endl;
            });
        }
    }

    void Run(uint32_t nWorkers = 0)
    {
        if(nWorkers == 0)
        {
            nWorkers = std::thread::hardware_concurrency();
        }

        tf::Executor executor(nWorkers);
        executor.run(m_taskflow).get();
		m_taskflow.dump(std::cout);
    }

protected:

    tf::Task* FindTask(TaskModule* pMod)
    {
        auto it = m_mapTasks.find(pMod);
        if(it != m_mapTasks.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    tf::Task* AddTask(TaskModule* pMod)
    {
        if(pMod->IsCondition())
        {
            m_mapTasks[pMod] = m_taskflow.emplace([=]() { return pMod->Run(); });
        }
        else
        {
            m_mapTasks[pMod] = m_taskflow.emplace([=]() { (void)pMod->Run(); });
        }

        tf::Task* curr = &m_mapTasks[pMod];
        curr->name(pMod->Name());
        
        switch(pMod->GetPriority())
        {
            case TaskPrio::HI:
                curr->priority(tf::TaskPriority::HIGH);
                break;
            case TaskPrio::NO:
                curr->priority(tf::TaskPriority::NORMAL);
                break;
            case TaskPrio::LO:
                curr->priority(tf::TaskPriority::LOW);
                break;
        }

        return curr;
    }

    void visit(TaskModule* pMod, std::set<TaskModule*>& visited, const std::function<void(TaskModule* mod, TaskModule* pPrev, TaskModule* pNext)>& lambda)
    {
        if (visited.count(pMod) > 0 || pMod == nullptr)
            return;

        visited.insert(pMod);

        // prev list
        for (auto& prev : pMod->GetPrevList())
        {
            lambda(pMod, prev, nullptr);
            visit(prev, visited, lambda);
        }

        // next list
        for (auto& next : pMod->GetNextList())
        {
            lambda(pMod, nullptr, next);
            visit(next, visited, lambda);
        }
    }

protected:
    std::unordered_map<TaskModule*, tf::Task> m_mapTasks;
    tf::Taskflow m_taskflow;
};

class InitGlobal
{
public:
    uint32_t Run()
    {
        std::cout << "Run class end\n";
        return 0;
    }
};

class StartZmqSvr
{
public:
    uint32_t Run()
    {
        std::cout << "Run class end\n";
        return 0;
    }
};

class ChkActState
{
public:
    uint32_t Run()
    {
        std::cout << "Run class end\n";
        return 0;
    }
};

class ChkOtaEvt
{
public:
    uint32_t Run()
    {
        std::cout << "Run class end\n";
        return 0;
    }
};

class Flash
{
public:
    uint32_t Run()
    {
        std::cout << "Run class end\n";
        return 0;
    }
};

class Activate
{
public:
    uint32_t Run()
    {
        std::cout << "Run class end\n";
        return 0;
    }
};

class EndFlash
{
public:
    uint32_t Run()
    {
        std::cout << "Run class end\n";
        return 1;
    }
};

class EndAct
{
public:
    uint32_t Run()
    {
        std::cout << "Run class end\n";
        return 0;
    }
};

class Reboot
{
public:
    uint32_t Run()
    {
        std::cout << "Run class end\n";
        return 0;
    }
};

void module()
{
    TModule<InitGlobal>        modInitGlobal;
    TModule<StartZmqSvr>       modStartZmqSvr;
    TModule<ChkActState>       modChkActState;
    TModule<ChkOtaEvt, true>   modChkOtaEvt;
    TModule<Flash>             modFlash;
    TModule<Activate>          modActivate;
    TModule<EndFlash, true>    modEndFlash;
    TModule<EndAct, true>      modEndAct;
    TModule<Reboot>            modReboot;

    TModule<Flash>             modFlash1;
    TModule<Flash>             modFlash2;

    modInitGlobal.Before({&modStartZmqSvr, &modChkActState});
	modChkActState >> &modChkOtaEvt.Before({&modFlash, &modActivate});
    modActivate >> &modEndAct.Before({&modReboot, &modChkOtaEvt});
    modFlash >> &modEndFlash >> &modChkOtaEvt;
    modFlash.After({&modFlash1, &modFlash2});

    modStartZmqSvr.SetPriority(TaskPrio::HI);

    TaskExecutor exec("OTA", {
            &modInitGlobal,
            
            /*&modStartZmqSvr,
            &modChkActState,
            &modChkOtaEvt,
            &modFlash,
            &modActivate,
            &modEndFlash,
            &modEndAct,
            &modReboot*/
    });

    exec.Run();
}

template<typename T>
void printValue(T value) {
    std::cout << "传入的值是：" << value << std::endl;
}

int main() {
    //printValue(5);
    module();
/*
    tf::Executor executor(5);
    tf::Taskflow taskflow("test");

    printf("threads: %d\n", executor.num_workers());

    for(int i=0; i<5; i++) {
        taskflow.emplace([=](){
                printf("thread:%ld\n", pthread_self());
                std::this_thread::sleep_for(std::chrono::seconds(i));
                });
    }

    // submit the taskflow
    tf::Future fu = executor.run(taskflow);

    // request to cancel the submitted execution above
    //fu.cancel();

    // wait until the cancellation finishes
    fu.get();

    taskflow.dump(std::cout);
*/
    return 0;
}

