/* =============================================================================
#     FileName: patrol.go
#         Desc:
#       Author: LiuYangming
#        Email: sdwhlym@126.com
#     HomePage: http://yummyliu.github.io
#      Version: 0.0.1
#   LastChange: 2018-07-27 13:06:07
#      History:
============================================================================= */
package main

import (
    "container/list"
    "database/sql"
    "flag"
    "fmt"
    _ "github.com/lib/pq"
    "io/ioutil"
    "log"
    "net/http"
    "strconv"
    "strings"
    "time"
)

type OpsMessage struct {
    OpsType       string
	metric		  float64
}
const (
	// cpu usage alert
	rangeLen = 60
	cpuThreshold = 50.0

	// active activity alert
	activeLen = 120
	activeThreshold = 80
)
var host     = "localhost"
var	port     = 5432
var	user     = "dba"
var	password = "dbapasswd"
var	dbname   = "postgres"
var connStr  = ""

func getCPUSample() (idle, total uint64) {
    contents, err := ioutil.ReadFile("/proc/stat")
    if err != nil {
        return
    }
    lines := strings.Split(string(contents), "\n")
    for _, line := range(lines) {
        fields := strings.Fields(line)
        if fields[0] == "cpu" {
            numFields := len(fields)
            for i := 1; i < numFields; i++ {
                val, err := strconv.ParseUint(fields[i], 10, 64)
                if err != nil {
                    log.Println("Error: ", i, fields[i], err)
                }
                total += val // tally up all the numbers to get total ticks
                if i == 4 {  // idle is the 5th field in the cpu line
                    idle = val
                }
            }
            return
        }
    }
    return
}

func cpuloadrate(idleTicks, totalTicks float64) float64 {
	return 100 * (totalTicks - idleTicks) / totalTicks
}

func cpuCheck(ops_ch chan<- OpsMessage) {
	log.Println("go cpu check")
	// stats collect
	sumCpuLoads := list.New()
	preidle, pretotal := getCPUSample()
	sumLoad := 0.0
	for  {
		time.Sleep(1 * time.Second)
    	curidle, curtotal := getCPUSample()

    	curcpuUsage := cpuloadrate(float64(curidle - preidle), float64(curtotal - pretotal))

		// put cur load into list and add it into sum
		sumLoad += curcpuUsage;
		sumCpuLoads.PushBack(curcpuUsage)

		if sumCpuLoads.Len() > rangeLen {
			if cur, ok := sumCpuLoads.Front().Value.(float64); ok {

				// remove oldest load outof list and cut it outof sum
				sumCpuLoads.Remove(sumCpuLoads.Front())
				sumLoad -= cur

				avgLoad := (sumLoad/float64(rangeLen))
				if avgLoad > cpuThreshold {
					log.Printf("danger!,CPU usage is %f%% avg usage is %f%%\n", curcpuUsage, avgLoad)
					ops_ch <- OpsMessage{OpsType: "killquery", metric: avgLoad}
				} else {
					log.Printf("safe~~~,CPU usage is %f%% avg usage is %f%%\n", curcpuUsage, avgLoad)
				}
			}
		}
	}
}

func activityCheck(ops_ch chan<- OpsMessage) {
	log.Println("go activity check")

	db, err := sql.Open("postgres", connStr)
	if err != nil {
		  panic(err)
	}
	defer db.Close()
	err = db.Ping()
	if err != nil {
		  panic(err)
	}

	counts := 0
	for  {
		time.Sleep(1 * time.Second)

		var count int
		err := db.QueryRow("select count(*) as count from pg_stat_activity where state = 'active';").Scan(&count)
		switch {
		case err == sql.ErrNoRows:
		        log.Printf("0 row")
		case err != nil:
		        log.Fatal(err)
		default:
			if count > activeThreshold{
				counts +=1
				if counts > activeLen {
					log.Printf("danger!,active count is %d%%\n", counts )
					ops_ch <- OpsMessage{OpsType: "killquery", metric: float64(count)}
				}
			} else {
				counts = 0
				log.Printf("safe~~~,count %d querys\n",count)
			}
		}
	}
}

func dbInfo(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "Hello World!")
}

func initFlag() {

	flag.StringVar(&host, "h", "localhost", "db host")
	flag.IntVar(&port, "p", 5432, "db port")
	flag.StringVar(&user, "u", "dba", "db user")
	flag.StringVar(&dbname,"d", "postgres", "monitor db")
	flag.StringVar(&password,"w", "123", "password of user")

	flag.Parse()
	connStr := fmt.Sprintf("host=%s port=%d user=%s password=%s dbname=%s sslmode=disable",
							host,
							port,
							user,
							password,
							dbname)
	log.Println(connStr)
}

func main() {

	initFlag()

    ops_ch := make(chan OpsMessage, 100)

	// connect to PostgreSQL
	db, err := sql.Open("postgres", connStr)
	if err != nil {
		  panic(err)
	}
	defer db.Close()
	err = db.Ping()
	if err != nil {
		  panic(err)
	}

	// patrol go on duty
	go cpuCheck(ops_ch)
	go activityCheck(ops_ch)
	log.Println("patrol go on duty!")

	// waiting for ops message
	for  {
		select {
		case msg := <-ops_ch:
			if msg.OpsType == "killquery" {
				res, err := db.Exec("select pg_cancel_backend(pid) from pg_stat_activity where pid <> pg_backend_pid() and usename != 'dba';")

				if err == nil {
					rows,_ := res.RowsAffected()
					log.Printf("metric: %f, kill %d querys\n",msg.metric, rows)
				}
			}
		}
	}
}
