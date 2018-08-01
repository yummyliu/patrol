package main

type DbTable struct {
	Tablename string
	Schemaname string
}

type DbInfo struct {
	Dbname string
	Maxage int64
	TopThreeAge [3]DbTable
}
