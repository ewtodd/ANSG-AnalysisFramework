{ stdenv, lib, cmake, ... }:

stdenv.mkDerivation {
  pname = "ANSG-AnalysisFramework";
  version = "0.1";

  src = ./.;

  nativeBuildInputs = [ cmake ];
  
  meta = with lib; {
    description = "A package to deploy C++ headers for ROOT";
    license = licenses.gpl3;  # Adjust based on your license
    platforms = platforms.unix;
  };
}
