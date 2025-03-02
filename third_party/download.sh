#! /bin/bash
set -eux

cd "$(dirname "$0")"

json_h_hash=$(cat json_h_hash.txt)
budoux_hash=$(cat budoux_hash.txt)

cd ..

rm -fR   _tmp
mkdir -p _tmp/staging

rm -fR   third_party/budoux
mkdir -p third_party/budoux/budoux/models
rm -fR   third_party/json.h
mkdir -p third_party/json.h

pushd _tmp/staging

curl -JLo sheredom-json.h.zip https://github.com/sheredom/json.h/archive/$json_h_hash.zip
unzip     sheredom-json.h.zip
mv json.h-$json_h_hash json.h

curl -JLo google-budoux.zip https://github.com/google/budoux/archive/$budoux_hash.zip
unzip     google-budoux.zip
mv budoux-$budoux_hash budoux

popd

cp _tmp/staging/json.h/LICENSE              third_party/json.h/
cp _tmp/staging/json.h/json.h               third_party/json.h/
cp _tmp/staging/budoux/LICENSE              third_party/budoux/
cp _tmp/staging/budoux/budoux/models/*.json third_party/budoux/budoux/models/

rm -fR _tmp/staging
