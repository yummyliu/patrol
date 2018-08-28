package main

import (
	"database/sql"
	"encoding/json"
	"net/http"
	"strconv"
	"strings"
	"syscall"
)

type DbInfo struct {
	Maxage int64
	DiskUsage float32
}

type DiskUsage struct {
		stat *syscall.Statfs_t
}

func NewDiskUsage(volumePath string) *DiskUsage {

	var stat syscall.Statfs_t
	syscall.Statfs(volumePath, &stat)
	return &DiskUsage{&stat}
}

// Total free bytes on file system
func (this *DiskUsage) Free() uint64 {
	return this.stat.Bfree * uint64(this.stat.Bsize)
}

// Total size of the file system
func (this *DiskUsage) Size() uint64 {
	return this.stat.Blocks * uint64(this.stat.Bsize)
}

// Total bytes used in file system
func (this *DiskUsage) Used() uint64 {
	return this.Size() - this.Free()
}

// Percentage of use on the file system
func (this *DiskUsage) Usage() float32 {
	return float32(this.Used()) / float32(this.Size())
}
func getDbAge(mdb *sql.DB) string {
	msql := `SELECT age(relfrozenxid)
	FROM pg_authid t1
	JOIN pg_class t2 ON t1.oid=t2.relowner
	JOIN pg_namespace t3 ON t2.relnamespace=t3.oid
	WHERE t2.relkind IN ($$t$$,$$r$$)
	ORDER BY age(relfrozenxid) DESC LIMIT 1;`

	var age string
	err := mdb.QueryRow(msql).Scan(&age)
	if err != nil && err != sql.ErrNoRows {
		log.Error(err)
	}

	return age
}

func getDiskUsage(mdb *sql.DB) float32 {
	msql := `show data_directory;`

	var dataDir string
	err := mdb.QueryRow(msql).Scan(&dataDir)
	if err != nil && err != sql.ErrNoRows {
		log.Error(err)
	}

	du := NewDiskUsage(dataDir)
	return du.Usage()
}

func getDbInfo(host string ) {
	connstr = "user=dba password=ttdbattdba sslmode=disable dbname=postgres" + " host="+host

	mdb, err := sql.Open("postgres", connstr)
	if err != nil {
		  log.Error(err)
	}
	err = mdb.Ping()
	if err != nil {
		  log.Error(err)
	}
	defer mdb.Close()

	msql := `SELECT datname
	FROM pg_database
	WHERE datname!='postgres'
		AND datname != 'template1'
	    AND datname != 'template0';`

	var dbname string
	err = mdb.QueryRow(msql).Scan(&dbname)
	if err != nil && err != sql.ErrNoRows {
		log.Error(err)
	}

	strings.Replace(connstr, " dbname=postgres", "dbname="+dbname,-1);
	ndb, err := sql.Open("postgres", connstr)
	if err != nil {
		  log.Error(err)
	}
	err = ndb.Ping()
	if err != nil {
		  log.Error(err)
	}

	ma,_ := strconv.ParseInt(getDbAge(ndb), 10, 64)
	dbinfo := &DbInfo{
		Maxage: ma,
		DiskUsage : getDiskUsage(ndb),
	}

	json, err := json.Marshal(dbinfo)
	if err != nil {
		panic(err)
	}

	w.Write(json)
}

func HttpServer() {

	server := http.Server{
		Addr: "0.0.0.0:8888",
	}
	http.HandleFunc("/dbinfo", getDbInfo)

	log.Info("http service on (8888)...")
	server.ListenAndServe()
}

