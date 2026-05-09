#include "HospitalConsole.h"
#include "HospitalService.h"

int main() {
    HospitalService service;
    HospitalConsole console(service);
    console.run();
    return 0;
}
