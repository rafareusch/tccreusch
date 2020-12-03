To run this code in the PUCRS provided kriti machine, please use the following commands in order to build and run the platform.

module load ovp/20170201

source /soft64/imperas/ferramentas/Imperas.20170201/bin/setup.sh

setupImperas /soft64/imperas/ferramentas/Imperas.20170201

git clone https://github.com/rafareusch/tccreusch.git

cd tccreusch

make build

make run


