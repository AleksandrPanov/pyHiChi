#pragma once
#include <string>
#include "Grid.h"
#include "ParticleArray.h"
#include <adios2.h>
namespace pfc {

#define put_var(x) PutVariable((#x), (x));
#define put_arr(x) PutArray((#x), (x));
#define get_arr(x) GetArray((#x), (x));
#define get_arr(x) GetArray((#x), (x));
enum class IOType { Undefined, Write, Read };
struct DataManager
{
    std::string path; //������ ����� ������ � ���� �� ������
    IOType mode = IOType::Undefined;

    adios2::ADIOS adios;
    // engine factory
    adios2::IO io;
    adios2::Engine engine;
    int iteration = 0;
public:
    DataManager(std::string path, IOType new_mode, int it = 0): path(path), iteration(it)
    {
        io = adios.DeclareIO("hiChiIO");
        setEngine(new_mode);
    }
    ~DataManager()
    {
        if (engine)
            engine.Close();
    }
    void setEngine(IOType new_mode)
    {
        if (mode != new_mode)
        {
            mode = new_mode;
            if (engine) engine.Close();
            engine = io.Open(path, (adios2::Mode)mode);
            io.SetEngine("BP4");
        }
    }
    void close()
    {
        //io.RemoveAllAttributes();
        //io.RemoveAllVariables();
        //io.ClearParameters();
        if (engine) 
            engine.Close();
    }
    void endStep() //flush buffer
    {
        engine.EndStep();
    }

    template<typename T>
    void putVariable(const std::string name, const T& val)
    {
        adios2::Variable<T> var = io.DefineVariable<T>(name);
        engine.Put(var, val);
    }
    template<typename T>
    void putArray(const std::string name, const T* arr, size_t size)
    {
        adios2::Variable<T> var = io.DefineVariable<T>(name, {},
                                  {}, { size }, adios2::ConstantDims);
        engine.Put(var, arr);
    }
    template<typename T>
    void getVariable(const std::string name, T& val)
    {
        engine.Get(name, val);
    }
    template<typename T>
    void getArray(const std::string name, T* arr, size_t size)
    {
        //auto varAr = io.InquireVariable<T>(name);
        //size_t sizeAr = varAr.SelectionSize();
        //engine.Get(varAr, arr);
        engine.Get(name, arr);
    }

    template<typename T>
    void customPut(const std::string name, const Vector1<T>& val)
    {
        putVariable(name + "x", val.x);
    }
    template<typename T>
    void customGet(const std::string name, Vector1<T>& val)
    {
        getVariable(name + "x", val.x);
    }

    template<typename T>
    void customPut(const std::string name, const Vector2<T>& val)
    {
        putVariable(name + "x", val.x);
        putVariable(name + "y", val.y);
    }
    template<typename T>
    void customGet(const std::string name, Vector2<T>& val)
    {
        getVariable(name + "x", val.x);
        getVariable(name + "y", val.y);
    }

    template<typename T>
    void customPut(const std::string name, const Vector3<T>& val)
    {
        putVariable(name + "x", val.x);
        putVariable(name + "y", val.y);
        putVariable(name + "z", val.z);
    }
    template<typename T>
    void customGet(const std::string name, Vector3<T>& val)
    {
        getVariable(name + "x", val.x);
        getVariable(name + "y", val.y);
        getVariable(name + "z", val.z);
    }

    template<typename Data>
    void customPut(const std::string name, ScalarField<Data> &field)
    {
        const int size = field.getSize().volume();
        customPut(name + "3dsize", field.getSize());
        putArray(name + "ar", field.getData(), size);
    }
    template<typename Data>
    void customGet(const std::string name, ScalarField<Data>& field)
    {
        Int3 size;
        customGet(name + "3dsize", size);
        field = ScalarField<Data>(size);
        Data* firstEl = field.getData();
        getArray(name + "ar", field.getData(), size.volume());
    }

    template<typename Data, GridTypes gridType>
    void customPut(const std::string name, const Grid<Data, gridType> &grid)
    {
        putVariable(name + "dt", grid.dt);
        customPut(name + "numInternalCells", grid.numInternalCells);
        customPut(name + "minCoord", grid.minCoord);
        customPut(name + "steps", grid.steps);
        customPut(name + "globalGridDims", grid.globalGridDims);
    }

    template<typename Data, GridTypes gridType>
    void customGet(const std::string name, Grid<Data, gridType>&grid)
    {
        FP dt;
        Int3 numInternalCells, globalGridDims;
        FP3 minCoord, steps;
        getVariable(name + "dt", dt);
        customGet(name + "numInternalCells", numInternalCells);
        customGet(name + "minCoord", minCoord);
        customGet(name + "steps", steps);
        customGet(name + "globalGridDims", globalGridDims);
        //grid = Grid<Data, gridType>(numInternalCells, dt, minCoord, steps, globalGridDims);
        //grid = YeeGrid(numInternalCells, dt, minCoord, steps, globalGridDims);
        //�������� ������������� ����
    }

    //Particle
    template<Dimension dimension>
    void customPut(const std::string name, Particle<dimension>& particle)
    {
        customPut("PositionType", particle.position);
        customPut("MomentumType", particle.p);
        putVariable("WeightType", particle.weight);
        putVariable("GammaType", particle.gamma);
        putVariable("TypeIndexType", (int)particle.typeIndex);
    }
    template<Dimension dimension>
    void customGet(const std::string name, Particle<dimension>& particle)
    {
    }

    //ParticleArrayAoS
    template<Dimension dimension>
    void customPut(const std::string name, ParticleArrayAoS<dimension>& particleArray)
    {
        const size_t size = particleArray.size();
        putVariable("size", size);
    }
    template<Dimension dimension>
    void customGet(const std::string name, ParticleArrayAoS<dimension>& particleArray)
    {
       size_t size;
       getVariable("size", size);
    }

    //ParticleArraySoA
    template<Dimension dimension>
    void customPut(const std::string name, ParticleArraySoA<dimension>& particleArray)
    {
        const size_t size = particleArray.size();
        putVariable("size", size);
    }
    template<Dimension dimension>
    void customGet(const std::string name, ParticleArraySoA<dimension>& particleArray)
    {
        size_t size;
        getVariable("size", size);
    }
};
} // namespace pfc