package main

import (
	"database/sql"
	_ "github.com/lib/pq"
	"log"
)

func main() {
	Db, err := sql.Open("postgres","user=liuyangming dbname=postgres sslmode=disable")
	if err != nil {
		panic(err)
	}

	err = Db.Ping()
	if err != nil {
		  panic(err)
	}


	rows,err := Db.Query("select 1 as a;")
	if err != nil {
		panic(err)
	}

	for rows.Next() {
		var a int
		rows.Scan(&a)
		log.Println(a)
	}
	rows.Close()
}
