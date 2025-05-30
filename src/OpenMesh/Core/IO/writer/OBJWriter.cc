/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2025, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */




//== INCLUDES =================================================================


//STL
#include <fstream>
#include <limits>

// OpenMesh
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/writer/OBJWriter.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>

//=== NAMESPACES ==============================================================


namespace OpenMesh {
namespace IO {


//=== INSTANCIATE =============================================================


// register the OBJLoader singleton with MeshLoader
_OBJWriter_  __OBJWriterinstance;
_OBJWriter_& OBJWriter() { return __OBJWriterinstance; }


//=== IMPLEMENTATION ==========================================================


_OBJWriter_::_OBJWriter_() { IOManager().register_module(this); }


//-----------------------------------------------------------------------------


bool
_OBJWriter_::
write(const std::string& _filename, BaseExporter& _be, const Options& _writeOptions, std::streamsize _precision) const
{
  std::fstream out(_filename.c_str(), std::ios_base::out );

  if (!out)
  {
    omerr() << "[OBJWriter] : cannot open file "
	  << _filename << std::endl;
    return false;
  }

  // Set precision on output stream. The default is set via IOManager and passed through to all writers.
  out.precision(_precision);

  // Set fixed output to avoid problems with programs not reading scientific notation correctly
  out << std::fixed;

  {
#if defined(_WIN32)
    std::string::size_type dotposition = _filename.find_last_of("\\/");
#else
    std::string::size_type dotposition = _filename.rfind("/");
#endif

    if (dotposition == std::string::npos){
      path_ = "./";
      objName_ = _filename;
    }else{
      path_ = _filename.substr(0,dotposition+1);
      objName_ = _filename.substr(dotposition+1);
    }

    //remove the file extension
    dotposition = objName_.find_last_of(".");

    if(dotposition != std::string::npos)
      objName_ = objName_.substr(0,dotposition);
  }

  bool result = write(out, _be, _writeOptions, _precision);

  out.close();
  return result;
}

//-----------------------------------------------------------------------------

size_t _OBJWriter_::getMaterial(OpenMesh::Vec3f _color) const
{
  auto idx_it = material_idx_.find(_color);
  if (idx_it != material_idx_.end()) {
      return idx_it->second;
  } else {
      size_t idx = material_.size();
      material_.push_back(_color);
      material_idx_[_color] = idx;

      return idx;
  }
}

//-----------------------------------------------------------------------------

size_t _OBJWriter_::getMaterial(OpenMesh::Vec4f _color) const
{
  auto idx_it = materialA_idx_.find(_color);
  if (idx_it != materialA_idx_.end()) {
      return idx_it->second;
  } else {
      size_t idx = materialA_.size();
      materialA_.push_back(_color);
      materialA_idx_[_color] = idx;
      return idx;
  }
}

//-----------------------------------------------------------------------------

bool
_OBJWriter_::
writeMaterial(std::ostream& _out, BaseExporter& _be, Options _opt) const
{
  OpenMesh::Vec3f c;
  OpenMesh::Vec4f cA;

  material_.clear();
  material_idx_.clear();
  materialA_.clear();
  materialA_idx_.clear();

  //iterate over faces
  for (size_t i=0, nF=_be.n_faces(); i<nF; ++i)
  {
    //color with alpha
    if ( _opt.color_has_alpha() ){
      cA  = color_cast<OpenMesh::Vec4f> (_be.colorA( FaceHandle(int(i)) ));
      getMaterial(cA);
    }else{
    //and without alpha
      c  = color_cast<OpenMesh::Vec3f> (_be.color( FaceHandle(int(i)) ));
      getMaterial(c);
    }
  }

  //write the materials
  if ( _opt.color_has_alpha() )
    for (size_t i=0; i < materialA_.size(); i++){
      _out << "newmtl " << "mat" << i << '\n';
      _out << "Ka 0.5000 0.5000 0.5000" << '\n';
      _out << "Kd " << materialA_[i][0] << ' ' << materialA_[i][1] << ' ' << materialA_[i][2] << '\n';
      _out << "Tr " << materialA_[i][3] << '\n';
      _out << "illum 1" << '\n';
    }
  else
    for (size_t i=0; i < material_.size(); i++){
      _out << "newmtl " << "mat" << i << '\n';
      _out << "Ka 0.5000 0.5000 0.5000" << '\n';
      _out << "Kd " << material_[i][0] << ' ' << material_[i][1] << ' ' << material_[i][2] << '\n';
      _out << "illum 1" << '\n';
    }

  if (_opt.texture_file != "") {
      _out << "map_Kd " << _opt.texture_file << std::endl;
  }
  return true;
}

//-----------------------------------------------------------------------------


bool
_OBJWriter_::
write(std::ostream& _out, BaseExporter& _be, const Options& _writeOptions, std::streamsize _precision) const
{
  unsigned int idx;
  VertexHandle vh;
  std::vector<VertexHandle> vhandles;
  bool useMatrial = false;
  OpenMesh::Vec3f c;
  OpenMesh::Vec4f cA;

  omlog() << "[OBJWriter] : write file\n";

  _out.precision(_precision);

  // check exporter features
  if (!check( _be, _writeOptions)) {
      return false;
  }


  // No binary mode for OBJ
  if ( _writeOptions.check(Options::Binary) ) {
    omout() << "[OBJWriter] : Warning, Binary mode requested for OBJ Writer (No support for Binary mode), falling back to standard." <<  std::endl;
  }

  // check for unsupported writer features
  if (_writeOptions.check(Options::FaceNormal) ) {
    omerr() << "[OBJWriter] : FaceNormal not supported by OBJ Writer" <<  std::endl;
    return false;
  }

  // check for unsupported writer features
  if (_writeOptions.check(Options::VertexColor) ) {
    omerr() << "[OBJWriter] : VertexColor not supported by OBJ Writer" <<  std::endl;
    return false;
  }

  //create material file if needed
  if ( _writeOptions.check(Options::FaceColor) || _writeOptions.texture_file != ""){

    std::string matFile = path_ + objName_ + _writeOptions.material_file_extension;

    std::fstream matStream(matFile.c_str(), std::ios_base::out );

    if (!matStream)
    {
      omerr() << "[OBJWriter] : cannot write material file " << matFile << std::endl;

    }else{
      useMatrial = writeMaterial(matStream, _be, _writeOptions);

      matStream.close();
    }
  }

  // header
  _out << "# " << _be.n_vertices() << " vertices, ";
  _out << _be.n_faces() << " faces" << '\n';

  // material file
  if ( (useMatrial &&  _writeOptions.check(Options::FaceColor)) || _writeOptions.texture_file != "")
    _out << "mtllib " << objName_ << _writeOptions.material_file_extension << '\n';

  std::map<Vec2f,int> texMap;
  //collect Texturevertices from halfedges
  if(_writeOptions.check(Options::FaceTexCoord))
  {
    std::vector<Vec2f> texCoords;
    //add all texCoords to map
    unsigned int num = _be.get_face_texcoords(texCoords);
    for(size_t i = 0; i < num ; ++i)
    {
      texMap[texCoords[i]] = static_cast<int>(i);
    }
  }

  //collect Texture coordinates from vertices
  if(_writeOptions.check(Options::VertexTexCoord))
  {
    for (size_t i=0, nV=_be.n_vertices(); i<nV; ++i)
    {
      vh = VertexHandle(static_cast<int>(i));
      Vec2f t  = _be.texcoord(vh);
      texMap[t] = static_cast<int>(i);
    }
  }

  // assign each texcoord in the map its id
  // and write the vt entries
  if(_writeOptions.check(Options::VertexTexCoord) || _writeOptions.check(Options::FaceTexCoord))
  {
    int texCount = 0;
    for(std::map<Vec2f,int>::iterator it = texMap.begin(); it != texMap.end() ; ++it)
    {
      _out << "vt " << it->first[0] << " " << it->first[1] << '\n';
      it->second = ++texCount;
    }
  }

  const bool normal_double = _be.is_normal_double();
  const bool point_double = _be.is_point_double();
  for (size_t i=0, nV=_be.n_vertices(); i<nV; ++i)
  {
      vh = VertexHandle(int(i));
      if (point_double) {
          auto v = _be.pointd(vh);
          _out << "v " << v[0] <<" "<< v[1] <<" "<< v[2] << '\n';
      } else {
          auto v = _be.point(vh);
          _out << "v " << v[0] <<" "<< v[1] <<" "<< v[2] << '\n';
      }
      if (_writeOptions.check(Options::VertexNormal)) {
        if (normal_double) {
            auto n = _be.normald(vh);
            _out << "vn " << n[0] <<" "<< n[1] <<" "<< n[2] << '\n';
        } else {
            auto n = _be.normal(vh);
            _out << "vn " << n[0] <<" "<< n[1] <<" "<< n[2] << '\n';
        }
      }
  }

  size_t lastMat = std::numeric_limits<std::size_t>::max();

  // we do not want to write seperators if we only write vertex indices
  bool onlyVertices =    !_writeOptions.check(Options::VertexTexCoord)
                      && !_writeOptions.check(Options::VertexNormal)
                      && !_writeOptions.check(Options::FaceTexCoord);

  // faces (indices starting at 1 not 0)
  for (size_t i=0, nF=_be.n_faces(); i<nF; ++i)
  {

    if (useMatrial &&  _writeOptions.check(Options::FaceColor) ){
      size_t material;

      //color with alpha
      if ( _writeOptions.color_has_alpha() ){
        cA  = color_cast<OpenMesh::Vec4f> (_be.colorA( FaceHandle(int(i)) ));
        material = getMaterial(cA);
      } else{
      //and without alpha
        c  = color_cast<OpenMesh::Vec3f> (_be.color( FaceHandle(int(i)) ));
        material = getMaterial(c);
      }

      // if we are ina a new material block, specify in the file which material to use
      if(lastMat != material) {
        _out << "usemtl mat" << material << '\n';
        lastMat = material;
      }
    }

    _out << "f";

    _be.get_vhandles(FaceHandle(int(i)), vhandles);

    for (size_t j=0; j< vhandles.size(); ++j)
    {

      // Write vertex index
      idx = vhandles[j].idx() + 1;
      _out << " " << idx;

      if (!onlyVertices) {
        // write separator
        _out << "/" ;

        //write texCoords index from halfedge
        if(_writeOptions.check(Options::FaceTexCoord))
        {
          _out << texMap[_be.texcoord(_be.getHeh(FaceHandle(int(i)),vhandles[j]))];
        }

        else
        {
          // write vertex texture coordinate index
          if (_writeOptions.check(Options::VertexTexCoord))
            _out  << texMap[_be.texcoord(vhandles[j])];
        }

        // write vertex normal index
        if ( _writeOptions.check(Options::VertexNormal) ) {
          // write separator
          _out << "/" ;
          _out << idx;
        }
      }
    }

    _out << '\n';
  }

  material_.clear();
  material_idx_.clear();
  materialA_.clear();
  materialA_idx_.clear();

  return true;
}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
