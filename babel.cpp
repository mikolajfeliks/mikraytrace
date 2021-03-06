#include "babel.h"

#include <openbabel/mol.h>
#include <openbabel/atom.h>
#include <openbabel/bond.h>
#include <openbabel/obiter.h>
#include <openbabel/obconversion.h>


namespace mrtp {

void create_molecule_tables(const std::string& mol2file,
                            std::vector<unsigned int>* atomic_nums,
                            std::vector<Eigen::Vector3d>* positions,
                            std::vector<std::pair<unsigned int, unsigned int>>* bonds) {
    OpenBabel::OBMol mol;
    OpenBabel::OBConversion conv;
    if(!conv.SetInFormat("mol2") || !conv.ReadFile(&mol, mol2file))
        return;

    FOR_ATOMS_OF_MOL(a, mol) {
        atomic_nums->push_back(a->GetAtomicNum());
        positions->push_back(Eigen::Vector3d{a->GetX(), a->GetY(), a->GetZ()});
    }

    FOR_BONDS_OF_MOL(b, mol) {
        bonds->push_back(std::pair<unsigned int, unsigned int>{
                    b->GetBeginAtomIdx() - 1, b->GetEndAtomIdx() - 1});
    }
}

}
