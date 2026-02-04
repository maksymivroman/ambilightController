#include <Arduino.h>

class Task {

public:
    Task() = default;
    explicit Task(bool startWith) {
        this->distinctChangeFlag = startWith;
    };

    template<typename FnTrue>
    void operator()(bool handler, FnTrue execute, bool skipTask = false){
        const bool force = checkForceExecute();
        if (((handler != this->distinctChangeFlag) && !skipTask) || force) {
            logger.log("[Task] Execute task handler (one time)");
            this->distinctChangeFlag = handler;
            if (handler || force) execute();
        }
    };

    template<typename FnTrue, typename FnFalse>
    void operator()(bool handler, FnTrue executeOnTrue, FnFalse executeOnFalse, bool skipTask = false){
        if (((handler != this->distinctChangeFlag) && !skipTask)) {
            logger.log("[Task] Execute task handler (conditional)");
            this->distinctChangeFlag = handler;
            handler ? executeOnTrue() : executeOnFalse();
        }
    };

    template<typename FnTrue, typename FnFalse, typename FnSkip>
    void operator()(bool handler, FnTrue executeOnTrue, FnFalse executeOnFalse, FnSkip skipTaskFn){
        const bool skip = skipTaskFn();
        if (((handler != this->distinctChangeFlag) && !skip)) {
            logger.log("[Task] Execute task handler (conditional, skip task)");
            this->distinctChangeFlag = handler;
            handler ? executeOnTrue() : executeOnFalse();
        }
    };

    /** Force execute only for one time task*/
    void executeNext(){
        this->executeOnNextCheck = true;
    };

private:
    bool distinctChangeFlag{false};
    bool executeOnNextCheck{false};

    bool checkForceExecute() {
        if (executeOnNextCheck) {
            this->executeOnNextCheck = false;
            return true;
        }
        return false;
    }

};

class IntervalTask {

public:
    template<typename Fn>
    void operator()(unsigned long interval, Fn execute, bool skipTask = false){
        unsigned long currentTime = millis();
        if ((currentTime >= this->executeTime) && !skipTask) {
//            logger.logSerial("[IntervalTask] Execute task handler (interval): ", interval);
            this->executeTime = currentTime + interval;
            execute();
        }
    };

private:
    unsigned long executeTime = 0;
};
