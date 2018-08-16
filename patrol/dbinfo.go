package main

import (
	"encoding/json"
	"net/http"
)

type DbTable struct {
	Tablename string
	Schemaname string
	Seqscan int64
	Indexscan int64
}

type DbFunc struct {
	Funcname string
	Schemaname string
	Total_call int64
	Total_time int64
}

type DbInfo struct {
	Dbname string
	Maxage int64
	TopThreeAge [3]DbTable
}

func getDbInfo(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")

	dbinfo := &DbInfo{
		Dbname: "postgres",
		Maxage: 1000000023,
		TopThreeAge: [3]DbTable{
			{Tablename: "t1", Schemaname: "s1"},
			{Tablename: "t2", Schemaname: "s2"},
			{Tablename: "t3", Schemaname: "s3"},
		},
	}

	json, err := json.Marshal(dbinfo)
	if err != nil {
		panic(err)
	}

	w.Write(json)
}

func HttpServer() {

	server := http.Server{
		Addr: "127.0.0.1:8888",
	}
	http.HandleFunc("/dbinfo", getDbInfo)

	log.Info("http service on (8888)...")
	server.ListenAndServe()
}
