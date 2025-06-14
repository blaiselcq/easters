#! /usr/bin/env bash

base=https://easters.dev
EASTERS_ID=$(<.id)

curl -s $base/"$1"/"$2"/input -H "Cookie: id=$EASTERS_ID"
