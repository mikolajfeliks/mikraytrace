#ifndef BABEL_H
#define BABEL_H

#include <string>
#include <vector>
#include <Eigen/Core>


namespace mrtp {

void create_molecule_tables(
        const std::string&,
        std::vector<unsigned int>*,
        std::vector<Eigen::Vector3d>*,
        std::vector<std::pair<unsigned int, unsigned int>>*
        );

}

#endif // BABEL_H
