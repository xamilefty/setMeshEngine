#include <iostream>
#include <math.h>
#include "structuredHexMesh.h"

#include "../maths/vectors.h"
/**
[INTRO] This is an attempt to create a class for structured mesh which has been created from sweep face
        it should be able to take a C4 elements of a structured mesh and the create a structured mesh from it

        The structured mesh is going to be initialized as void and then if the function sweep is called,
        the structure will be created from an input face

        This structure has dynamic allocated memory, since it is able to perform many sweep operations which,
        will then increase the number of elements. For that reason the Population variables should be private entities
        where the functions that can eventually increase the number of any kind of elements of the mesh updates the value.

[SWEEP] The desing of this operation is based on take the first addess of vec3 and perform all the operations till target is reached,
        then the address pointer increases++ one unit to next vec3 of the stack and the operation is performed recursively until
        the last point of the mesh surface is reached.

    - MEMORY MANAGEMENT:

        When a struturedMesh is instanciated a void C8 mesh is created, then if some operation is performed like sweep face,
        the proper function receive as a parameters a mesh surface and dynamically create the structuredHexMesh of the object.

        So Sweep Faces must call to a function inside its class to instance new memory allocation with the exact size needed by that operation.

    - EXPECTED PARAMETERS:

        Mesh face: pointer to the first vec3 allocated in the stack
        Sweep op : Target of the sweep operation and number of layers

    - RETURNED VALUES:

        Each time a sweep operation over a vertex is done the resulted vertex will be addressed somewhere in the stack were it should be
        according to the map of the structured mesh desinged in the documentation.
        The reason why is not allocated inmediatly is "BY DESING" because of the map created to allocate the strutured vertex of the mesh.


[GET]   This function receives the index of an element and gets the memory address of vertex of the stencil. Provably this function will
        be separated into many functions as center of cell faces, centroid, normals, adjacent centroids, volumes, areas.

[MEMBERS]

        vec3  Mesh
        vec3* Vertex
        vec3* Edges
        vec3* Faces


**/


structuredHexMesh::structuredHexMesh()

    :   m_H(0), m_W(0), m_L(0),
        m_Vertex_Population(0), m_Face_Population(0), m_Volume_Population(0), m_id(0),
        m_Total_Volume(0.l), m_cell_Volume(0.l),
        Mesh(), Centroids(), Vertex(), Edge_i(), Edge_j(), Tris(), Quad()

        {
            std::cout<<"[CONSTRUCTOR]: Structured Mesh"<<std::endl;
        }

/// COMENTADO HASTA VER QUE HAGO CON ESTO; QUIZAS LO QUITE PARA ACCEDER A LA MALLA SOLO A TRAVES DE LAS FUNCIONES DE GENERACION
//    structuredHexMesh::structuredHexMesh(const int h, const int w, const int l, const int count_mesh, const int  count_faces, const int count_volumes)
//
//        :   m_H(h), m_W(w), m_L(l),
//            m_Vertex_Population(count_mesh), m_Face_Population(count_faces), m_Volume_Population(count_volumes), m_id(0),
//            m_Total_Volume(0.0f), m_cell_Volume(0.0f),
//            Mesh(new vec3[count_mesh]),
//            Centroids(new vec3[count_volumes]),
//            Vertex(), Edge_i(), Edge_j(), Tris(), Quad()
//
//        {
//            std::cout<<"[CONSTRUCTOR]: Structured Mesh"<<std::endl;
//            std::cout<<"m_Vertex_Population: "<<m_Vertex_Population<<std::endl;
//            std::cout<<"MESH               : "<<&Mesh<<std::endl;
//            std::cout<<"Centroids          : "<<&Centroids<<std::endl;
//            std::cout<<"Vertex             : "<<Vertex<<std::endl;
//            std::cout<<"Edge_i             : "<<Edge_i<<std::endl;
//            std::cout<<"Edge_j             : "<<Edge_j<<std::endl;
//        }

    structuredHexMesh::~structuredHexMesh()
    {
        std::cout<<"\n[DESTRUCTOR]: Structured Mesh\n"<<std::endl;
//        delete[] Mesh;
//        delete[] Centroids;
    }

    void structuredHexMesh::DisplaceMesh(const vec3& offset)
    {
        for(unsigned i = 0; i<m_Vertex_Population; i++) Mesh[i].Add(offset);
    }

    void structuredHexMesh::RotateXMesh(const float &alphax)
    {
        for(unsigned i = 0; i<m_Vertex_Population; i++) Mesh[i]= vec3::RotX(Mesh[i], alphax);
    }

    void structuredHexMesh::RotateYMesh(const float &alphay)
    {
        for(unsigned i = 0; i<m_Vertex_Population; i++) Mesh[i]= vec3::RotY(Mesh[i], alphay);
    }

    void structuredHexMesh::RotateZMesh(const float &alphaz)
    {
        for(unsigned i = 0; i<m_Vertex_Population; i++) Mesh[i]= vec3::RotZ(Mesh[i], alphaz);
    }

    void structuredHexMesh::getCellConnections(const unsigned &id)
    {
        m_id = id;
        int HW = m_H*m_W;                       //Count vertex in a layer
        int count_Cells_layer = (m_H-1)*(m_W-1);//Count elements in a layer

        int count_rows_passed = 0;              //Adds the last vertex of the row |-0-|-1-|-2-|-3-(|) <-this
        int count_layers_passed = 0;            //Adds the top row of vertex-W

        count_rows_passed = id/(m_W-1);
        count_layers_passed = id/count_Cells_layer;

        int i = id + count_rows_passed + count_layers_passed * m_W ;

        this->Vertex[0] = &Mesh[i];
        this->Vertex[1] = &Mesh[i+1];
        this->Vertex[2] = &Mesh[i+1+m_W];
        this->Vertex[3] = &Mesh[i+m_W];

        this->Vertex[4] = &Mesh[i+HW];
        this->Vertex[5] = &Mesh[i+1+HW];
        this->Vertex[6] = &Mesh[i+1+m_W+HW];
        this->Vertex[7] = &Mesh[i+m_W+HW];

        int map_v2e_i[] = {0,1,2,3,4,7,6,5,0,1,2,3};
        int map_v2e_j[] = {1,2,3,0,7,6,5,4,4,5,6,7};

        for(int l = 0; l<12; l++)
        {
            this->Edge_i[l] = Vertex[map_v2e_i[l]];
            this->Edge_j[l] = Vertex[map_v2e_j[l]];
        }

    }


    long double structuredHexMesh::AreaC3(const vec3& p1, const vec3& p2, const vec3& p3)
    {
        vec3 W = vec3::Cross(p1,p2,p3);
        return 0.5l*sqrtl(W.x*W.x + W.y*W.y + W.z*W.z);
    }

/**
    float structuredHexMesh::AreaC4(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& v3)
        {
        vec3 W0,W1;
        W0.Cross(v0,v1,v3);
        W1.Cross(v2,v3,v1);

        return 0.5f*(sqrt(W0.x*W0.x+W0.y*W0.y+W0.z*W0.z)+sqrt(W1.x*W1.x+W1.y*W1.y+W1.z*W1.z));
        }
**/
    vec3 structuredHexMesh::CenterC3(const vec3& p1, const vec3& p2, const vec3& p3)
        {
            long double i = 1.l/3.l; //Multiplications are most efficient than divisions

            float x = (p1.x + p2.x + p3.x) * i;
            float y = (p1.y + p2.y + p3.y) * i;
            float z = (p1.z + p2.z + p3.z) * i;

            return vec3(x,y,z);
        }


    vec3 structuredHexMesh::CenterT4(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& v3)
    {
            long double i = 1.l/4.l; //Multiplications are most efficient than divisions
            long double x = (v0.x + v1.x + v2.x + v3.x) * i;
            long double y = (v0.y + v1.y + v2.y + v3.y) * i;
            long double z = (v0.z + v1.z + v2.z + v3.z) * i;

            return vec3(x,y,z);
    }

    long double structuredHexMesh::VolumeT4(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& v3)
    {
        long double v1v2v3 = vec3::DotCross(v0,v1,v2,v3);
        return (sqrtl(v1v2v3*v1v2v3) / 6.l);
    }

    vec3 structuredHexMesh::CenterC4(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& v3)
        {

        ///LA FUNCION AREAC3 está anulada porque produce una pequeña inestabilidad al usarlo en la media ponderada del centro de la cara
            vec3 C1 = structuredHexMesh::CenterC3(v0,v1,v2);
            long double ac1 = structuredHexMesh::AreaC3(v0,v1,v2);

            vec3 C2 = structuredHexMesh::CenterC3(v0,v2,v3);
            long double ac2 = structuredHexMesh::AreaC3(v0,v2,v3);

            vec3 D1 = structuredHexMesh::CenterC3(v0,v1,v3);
            long double ad1 = structuredHexMesh::AreaC3(v0,v1,v3);

            vec3 D2 = structuredHexMesh::CenterC3(v1,v2,v3);
            long double ad2 = structuredHexMesh::AreaC3(v1,v2,v3);

            long double x1,y1,z1,x2,y2,z2;

            x1 = (C1.x*ac1+C2.x*ac2)/(ac1+ac2);
            y1 = (C1.y*ac1+C2.y*ac2)/(ac1+ac2);
            z1 = (C1.z*ac1+C2.z*ac2)/(ac1+ac2);

            x2 = (D1.x*ad1+D2.x*ad2)/(ad1+ad2);
            y2 = (D1.y*ad1+D2.y*ad2)/(ad1+ad2);
            z2 = (D1.z*ad1+D2.z*ad2)/(ad1+ad2);

            long double x = 0.5l*(x1+x2);
            long double y = 0.5l*(y1+y2);
            long double z = 0.5l*(z1+z2);

            return vec3(x,y,z);
        }

    void structuredHexMesh::CenterC8()
        {
            vec3 v0 = *Vertex[0];
            vec3 v1 = *Vertex[1];
            vec3 v2 = *Vertex[2];
            vec3 v3 = *Vertex[3];
            vec3 v4 = *Vertex[4];
            vec3 v5 = *Vertex[5];
            vec3 v6 = *Vertex[6];
            vec3 v7 = *Vertex[7];

            vec3  cL = structuredHexMesh::CenterC4(v0,v1,v2,v3);
//            float aL = structuredHexMesh::AreaC4(v0,v1,v2,v3);
            vec3  cR = structuredHexMesh::CenterC4(v4,v7,v6,v5);
//            float aR = structuredHexMesh::AreaC4(v4,v7,v6,v5);
            vec3  cB = structuredHexMesh::CenterC4(v0,v3,v7,v4);
//            float aB = structuredHexMesh::AreaC4(v0,v3,v7,v4);
            vec3  cF = structuredHexMesh::CenterC4(v1,v5,v6,v2);
//            float aF = structuredHexMesh::AreaC4(v1,v5,v6,v2);
            vec3  cS = structuredHexMesh::CenterC4(v0,v4,v5,v1);
//            float aS = structuredHexMesh::AreaC4(v0,v4,v5,v1);
            vec3  cN = structuredHexMesh::CenterC4(v2,v6,v7,v3);
//            float aN = structuredHexMesh::AreaC4(v2,v6,v7,v3);

            vec3 Shell_centroid;

            long double i = 1.l/6.l;

            Shell_centroid.x = (cL.x + cR.x + cB.x + cF.x + cS.x + cN.x) * i;
            Shell_centroid.y = (cL.y + cR.y + cB.y + cF.y + cS.y + cN.y) * i;
            Shell_centroid.z = (cL.z + cR.z + cB.z + cF.z + cS.z + cN.z) * i;

            vec3 p0 = structuredHexMesh::CenterT4(Shell_centroid,v0,v1,v2);
            long double ap0 = structuredHexMesh::VolumeT4(Shell_centroid,v0,v1,v2);

            vec3 p1 = structuredHexMesh::CenterT4(Shell_centroid,v2,v3,v0);
            long double ap1 = structuredHexMesh::VolumeT4(Shell_centroid,v2,v3,v0);

            vec3 p2 = structuredHexMesh::CenterT4(Shell_centroid,v2,v5,v1);
            long double ap2 = structuredHexMesh::VolumeT4(Shell_centroid,v2,v5,v1);

            vec3 p3 = structuredHexMesh::CenterT4(Shell_centroid,v2,v6,v5);
            long double ap3 = structuredHexMesh::VolumeT4(Shell_centroid,v2,v6,v5);

            vec3 p4 = structuredHexMesh::CenterT4(Shell_centroid,v5,v6,v7);
            long double ap4 = structuredHexMesh::VolumeT4(Shell_centroid,v5,v6,v7);

            vec3 p5 = structuredHexMesh::CenterT4(Shell_centroid,v7,v4,v5);
            long double ap5 = structuredHexMesh::VolumeT4(Shell_centroid,v7,v4,v5);

            vec3 p6 = structuredHexMesh::CenterT4(Shell_centroid,v7,v3,v0);
            long double ap6 = structuredHexMesh::VolumeT4(Shell_centroid,v7,v3,v0);

            vec3 p7 = structuredHexMesh::CenterT4(Shell_centroid,v0,v4,v7);
            long double ap7 = structuredHexMesh::VolumeT4(Shell_centroid,v0,v4,v7);

            vec3 p8 = structuredHexMesh::CenterT4(Shell_centroid,v3,v2,v7);
            long double ap8 = structuredHexMesh::VolumeT4(Shell_centroid,v3,v2,v7);

            vec3 p9 = structuredHexMesh::CenterT4(Shell_centroid,v7,v2,v6);
            long double ap9 = structuredHexMesh::VolumeT4(Shell_centroid,v7,v2,v6);

            vec3 p10 = structuredHexMesh::CenterT4(Shell_centroid,v1,v5,v0);
            long double ap10 = structuredHexMesh::VolumeT4(Shell_centroid,v1,v5,v0);

            vec3 p11 = structuredHexMesh::CenterT4(Shell_centroid,v0,v5,v4);
            long double ap11 = structuredHexMesh::VolumeT4(Shell_centroid,v0,v5,v4);


            vec3 s0 = structuredHexMesh::CenterT4(Shell_centroid,v0,v1,v3);
            long double as0 = structuredHexMesh::VolumeT4(Shell_centroid,v0,v1,v3);

            vec3 s1 = structuredHexMesh::CenterT4(Shell_centroid,v1,v2,v3);
            long double as1 = structuredHexMesh::VolumeT4(Shell_centroid,v1,v2,v3);

            vec3 s2 = structuredHexMesh::CenterT4(Shell_centroid,v1,v5,v6);
            long double as2 = structuredHexMesh::VolumeT4(Shell_centroid,v1,v5,v6);

            vec3 s3 = structuredHexMesh::CenterT4(Shell_centroid,v1,v6,v2);
            long double as3 = structuredHexMesh::VolumeT4(Shell_centroid,v1,v6,v2);

            vec3 s4 = structuredHexMesh::CenterT4(Shell_centroid,v4,v5,v6);
            long double as4 = structuredHexMesh::VolumeT4(Shell_centroid,v4,v5,v6);

            vec3 s5 = structuredHexMesh::CenterT4(Shell_centroid,v4,v6,v7);
            long double as5 = structuredHexMesh::VolumeT4(Shell_centroid,v4,v5,v7);

            vec3 s6 = structuredHexMesh::CenterT4(Shell_centroid,v0,v3,v4);
            long double as6 = structuredHexMesh::VolumeT4(Shell_centroid,v0,v3,v4);

            vec3 s7 = structuredHexMesh::CenterT4(Shell_centroid,v3,v7,v4);
            long double as7 = structuredHexMesh::VolumeT4(Shell_centroid,v3,v7,v4);

            vec3 s8 = structuredHexMesh::CenterT4(Shell_centroid,v2,v6,v3);
            long double as8 = structuredHexMesh::VolumeT4(Shell_centroid,v2,v6,v3);

            vec3 s9 = structuredHexMesh::CenterT4(Shell_centroid,v3,v6,v7);
            long double as9 = structuredHexMesh::VolumeT4(Shell_centroid,v3,v6,v7);

            vec3 s10 = structuredHexMesh::CenterT4(Shell_centroid,v0,v1,v4);
            long double as10 = structuredHexMesh::VolumeT4(Shell_centroid,v0,v1,v4);

            vec3 s11 = structuredHexMesh::CenterT4(Shell_centroid,v1,v5,v4);
            long double as11 = structuredHexMesh::VolumeT4(Shell_centroid,v1,v5,v4);

//            std::cout<<"vol 0:"<<ap0<<std::endl;
//            std::cout<<"vol 1:"<<ap1<<std::endl;
//            std::cout<<"vol 2:"<<ap2<<std::endl;
//            std::cout<<"vol 3:"<<ap3<<std::endl;
//            std::cout<<"vol 4:"<<ap4<<std::endl;
//            std::cout<<"vol 5:"<<ap5<<std::endl;
//            std::cout<<"vol 6:"<<ap6<<std::endl;
//            std::cout<<"vol 7:"<<ap7<<std::endl;
//            std::cout<<"vol 8:"<<ap8<<std::endl;
//            std::cout<<"vol 9:"<<ap9<<std::endl;
//            std::cout<<"vol 10:"<<ap10<<std::endl;
//            std::cout<<"vol 11:"<<ap11<<std::endl;

//            std::cout<<"vol 0:"<<as0<<std::endl;
//            std::cout<<"vol 1:"<<as1<<std::endl;
//            std::cout<<"vol 2:"<<as2<<std::endl;
//            std::cout<<"vol 3:"<<as3<<std::endl;
//            std::cout<<"vol 4:"<<as4<<std::endl;
//            std::cout<<"vol 5:"<<as5<<std::endl;
//            std::cout<<"vol 6:"<<as6<<std::endl;
//            std::cout<<"vol 7:"<<as7<<std::endl;
//            std::cout<<"vol 8:"<<as8<<std::endl;
//            std::cout<<"vol 9:"<<as9<<std::endl;
//            std::cout<<"vol 10:"<<as10<<std::endl;
//            std::cout<<"vol 11:"<<as11<<std::endl;

            m_cell_Volume = ap0+ap1+ap2+ap3+ap4+ap5+ap6+ap7+ap8+ap9+ap10+ap11;

//            long double Vol = v0.DotCross(v0,v1,v2,v3);
//            std::cout<<"APROXIMATED VOLUME: "<<sqrtl(Vol*Vol)<<std::endl;
//            std::cout<<"EXACT       VOLUME: "<<m_cell_Volume<<std::endl;
//            m_cell_Volume = as0+as1+as2+as3+as4+as5+as6+as7+as8+as9+as10+as11;

            m_Total_Volume += m_cell_Volume;

vec3 centerP;
            centerP.x = (p0.x*ap0 + p1.x*ap1 + p2.x*ap2 + p3.x*ap3 + p4.x*ap4 + p5.x*ap5 + p6.x*ap6 + p7.x*ap7 + p8.x*ap8 + p9.x*ap9 + p10.x*ap10 + p11.x*ap11)/
                                (     ap0 +      ap1 +      ap2 +      ap3 +      ap4 +      ap5 +      ap6 +      ap7 +      ap8 +      ap9 +       ap10 +       ap11);
            centerP.y = (p0.y*ap0 + p1.y*ap1 + p2.y*ap2 + p3.y*ap3 + p4.y*ap4 + p5.y*ap5 + p6.y*ap6 + p7.y*ap7 + p8.y*ap8 + p9.y*ap9 + p10.y*ap10 + p11.y*ap11)/
                                (     ap0 +      ap1 +      ap2 +      ap3 +      ap4 +      ap5 +      ap6 +      ap7 +      ap8 +      ap9 +       ap10 +       ap11);
            centerP.z = (p0.z*ap0 + p1.z*ap1 + p2.z*ap2 + p3.z*ap3 + p4.z*ap4 + p5.z*ap5 + p6.z*ap6 + p7.z*ap7 + p8.z*ap8 + p9.z*ap9 + p10.z*ap10 + p11.z*ap11)/
                                (     ap0 +      ap1 +      ap2 +      ap3 +      ap4 +      ap5 +      ap6 +      ap7 +      ap8 +      ap9 +       ap10 +       ap11);

vec3 centerS;
            centerS.x = (s0.x*as0 + s1.x*as1 + s2.x*as2 + s3.x*as3 + s4.x*as4 + s5.x*as5 + s6.x*as6 + s7.x*as7 + s8.x*as8 + s9.x*as9 + s10.x*as10 + s11.x*as11)/
                                (     as0 +      as1 +      as2 +      as3 +      as4 +      as5 +      as6 +      as7 +      as8 +      as9 +       as10 +       as11);
            centerS.y = (s0.y*as0 + s1.y*as1 + s2.y*as2 + s3.y*as3 + s4.y*as4 + s5.y*as5 + s6.y*as6 + s7.y*as7 + s8.y*as8 + s9.y*as9 + s10.y*as10 + s11.y*as11)/
                                (     as0 +      as1 +      as2 +      as3 +      as4 +      as5 +      as6 +      as7 +      as8 +      as9 +       as10 +       as11);
            centerS.z = (s0.z*as0 + s1.z*as1 + s2.z*as2 + s3.z*as3 + s4.z*as4 + s5.z*as5 + s6.z*as6 + s7.z*as7 + s8.z*as8 + s9.z*as9 + s10.z*as10 + s11.z*as11)/
                                (     as0 +      as1 +      as2 +      as3 +      as4 +      as5 +      as6 +      as7 +      as8 +      as9 +       as10 +       as11);
long double p = 0.l;
long double s = 1.l-p;

Centroids[m_id].x = (s*centerS.x + p*centerP.x);
Centroids[m_id].y = (s*centerS.y + p*centerP.y);
Centroids[m_id].z = (s*centerS.z + p*centerP.z);

//Centroids[m_id].x = (p0.x + p1.x + p2.x + p3.x + p4.x + p5.x + p6.x + p7.x + p8.x + p9.x + p10.x + p11.x)/(12.0f);
//Centroids[m_id].y = (p0.y + p1.y + p2.y + p3.y + p4.y + p5.y + p6.y + p7.y + p8.y + p9.y + p10.y + p11.y)/(12.0f);
//Centroids[m_id].z = (p0.z + p1.z + p2.z + p3.z + p4.z + p5.z + p6.z + p7.z + p8.z + p9.z + p10.z + p11.z)/(12.0f);

//Centroids[m_id] = Shell_centroid;

//            Centroids[1] = cL;
//            Centroids[2] = cR;
//            Centroids[3] = cB;
//            Centroids[4] = cF;
//            Centroids[5] = cS;
//            Centroids[6] = cN;
//            Centroids[7] = Shell_centroid;
//            Centroids[8] = Shell_centroid;
//            Centroids[9] = Shell_centroid;
//            Centroids[10] = Shell_centroid;
//            Centroids[11] = Shell_centroid;
//            Centroids[12] = Shell_centroid;

}

void structuredHexMesh::setHexMeshElement(const vec3* vertex_data_C8)
{
    if(Mesh==0)
    {
        Mesh        = new vec3[8];
        Centroids   = new vec3[13];

        m_H = 2;
        m_W = 2;

        m_id = 0;

        for(int i=0; i<8; i++)
        {
            Mesh[i] = *(vertex_data_C8 + i);
            m_Vertex_Population++;
        }

        m_Face_Population   += 6;
        m_Volume_Population += 1;

        m_Total_Volume = 0.0l;
        m_cell_Volume = 0.0l;
//        Vertex(), Edge_i(), Edge_j(), Tris(), Quad()
    }

    else
    {
        std::cout<<"Mesh has been already created"<<std::endl;
    }

}


    void structuredHexMesh::SweepFace(vec3* MeshSurface, int c_Xwide, int c_Zheight, int c_Ysweep, long double sweepValue, const vec3 &origin)
    {
    if(Mesh==0)
    {
        m_H = c_Zheight;
        m_W = c_Xwide;
        m_id = (m_H-1)*(m_W-1)*c_Ysweep;

        m_Vertex_Population = (c_Ysweep+1)*m_H*m_W;
        m_Volume_Population = (m_H-1)*(m_W-1)*c_Ysweep;

        int count_MeshSurface = m_H*m_W;


        Mesh        = new vec3[(c_Ysweep+1)*count_MeshSurface];
        Centroids   = new vec3[c_Ysweep*(m_H-1)*(m_W-1)];

        for(int i=0; i<count_MeshSurface; i++)
        {
            Mesh[i] = *(MeshSurface+i);

            for (int j = 0; j<c_Ysweep; j++)
            {

	//Extrude a mesh around Z-axis
    	//	Mesh[i+(j+1)*count_MeshSurface] = vec3::Loc2Glob(vec3::RotZ(vec3::Glob2Loc(Mesh[i],origin),(j+1)*sweepValue),origin);
		Mesh[i+(j+1)*count_MeshSurface] = RotZ(Mesh[i],sweepValue);
		std::cout<<"ROTZ"<<std::endl;
		//Extrude a mesh around Y-axis
		//Mesh[i+(j+1)*count_MeshSurface] = vec3::Loc2Glob(vec3::RotY(vec3::Glob2Loc(Mesh[i],origin),(j+1)*sweepValue),origin);

	//Extrude a mesh along a vector, this operation can be combined with the previous one
		//Mesh[i+(j+1)*count_MeshSurface].Add(vec3(0.l, 1.l*(j+1), 0.l));
		//Mesh[i+(j+1)*count_MeshSurface] = vec3::Loc2Glob(vec3::RotY(vec3::Glob2Loc(Mesh[i+(j+1)*count_MeshSurface],origin),(j+1)*sweepValue),origin);
            }
        }

        std::cout<<"\n--------------------------------[SWEEP FACE]"<<std::endl;
        std::cout<<"\tVertex\tFaces\tVolumes"<<std::endl;
    }

    else
    {
        std::cout<<"Mesh has been already created"<<std::endl;
    }
    }

void structuredHexMesh::Log()
{
    std::cout<<"\n----------------------------------[LOG MESH]"<<std::endl;
    std::cout<<"\tVertex\tFaces\tVolumes\tVolume"<<std::endl;
    std::cout<<"Size:\t"<<m_Vertex_Population<<"\t"<<m_Face_Population<<"\t"<<m_Volume_Population<<"\t"<<m_Total_Volume<<std::endl;
    std::cout<<"\nm_ID:\t"<<m_id<<std::endl;

    bool log_mesh_vertex = true;
    if(log_mesh_vertex)
    {
        std::cout<<"Vertex\tID \tX Y Z Coordinates\n";
        for(unsigned i = 0; i<8; i++)
        {
            std::cout<<"\tID "<<i<<"\t"<<*Vertex[i]<<std::endl;
        }
    }
}

unsigned structuredHexMesh::Log_Cells(unsigned id)
{
    if(id<m_Volume_Population)
    {
        return id;
    }
    else
    {
        return m_Volume_Population - 1;
    }
}

unsigned structuredHexMesh::Log_Cells_Max()
{
    return m_Volume_Population;
}

unsigned structuredHexMesh::Log_Verts()
{
    return m_Vertex_Population;
}