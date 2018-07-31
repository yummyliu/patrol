package main

type DbTable struct {
	tablename string
	schemaname string
}

type DbInfo struct {
	dbname string
	maxage string
	topThreeAge [3]DbTable
}
