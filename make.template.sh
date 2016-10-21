sed 's/$/\\n/' src/generater/pbdcex.core.hxx | tr -d '\r' | tr -d '\n' | sed 's/\"/\\\"/g' > pbdcex.core.string
