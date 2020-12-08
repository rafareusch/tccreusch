To run this code in the PUCRS provided kriti machine, please use the following commands in order to build and run the platform.

module load ovp/20170201

source /soft64/imperas/ferramentas/Imperas.20170201/bin/setup.sh

setupImperas /soft64/imperas/ferramentas/Imperas.20170201

git clone https://github.com/rafareusch/tccreusch.git

cd tccreusch

make build

make run


###################################
To run with reports enabled, insted of `make run` use:

harness.exe --verbose --modulefile platform/model.so --program RScpu=application/secureApplication.ARM_CORTEX_A8.elf  --program cpuRNS1=application/nonSecureApplication1.ARM_CORTEX_A8.elf  --program cpuRNS2=application/nonSecureApplication2.ARM_CORTEX_A8.elf




