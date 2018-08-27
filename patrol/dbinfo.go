package main

import (
	"encoding/json"
	"net/http"
	"strconv"
	"syscall"
)

type DiskUsage struct {
		stat *syscall.Statfs_t
}

func NewDiskUsage(volumePath string) *DiskUsage {

	var stat syscall.Statfs_t
	syscall.Statfs(volumePath, &stat)
	return &DiskUsage{&stat}
}

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
	Maxage int64
	DiskUsage float32
}
// Total free bytes on file system
func (this *DiskUsage) Free() uint64 {
	return this.stat.Bfree * uint64(this.stat.Bsize)
}

// Total available bytes on file system to an unpriveleged user
func (this *DiskUsage) Available() uint64 {
	return this.stat.Bavail * uint64(this.stat.Bsize)
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

func getDbInfo(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")

	ma,_ := strconv.ParseInt(getDbAge(), 10, 64)

	dbinfo := &DbInfo{
		Maxage: ma,
		DiskUsage : getDiskUsage(),
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

