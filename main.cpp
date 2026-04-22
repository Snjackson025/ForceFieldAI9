#include <iostream>
#include "ForceFieldAI.h"

int main()
{
    try
    {
        ForceFieldAI ai(5.0);
        ai.addSource(VectorND{0.0, 0.0, 0.0});
        ai.addSource(VectorND{10.0, 0.0, 0.0});

        VectorND position{2.0, 1.0, 0.0};
        VectorND result = ai.computeNetForce(position);

        std::cout << "Net Force: ";
        for (size_t i = 0; i < result.dimension(); ++i) std::cout << result[i] << " ";
        std::cout << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
    }

    return 0;
}