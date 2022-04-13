# The path to the executable should be given as the first argument to this script
echo "-------------------- Illustrative example --------------------"
$1/static_task_scheduling \
-c ./data/example_cluster.csv \
-t ./data/example_task_bags.csv \
-d ./data/example_dependencies.csv
echo "-------------------- Epigenome small --------------------"
$1/static_task_scheduling \
-c ./data/small_cluster.csv \
-t ./data/epigenome_100.csv \
-p epigenome
echo "-------------------- Epigenome large --------------------"
$1/static_task_scheduling \
-c ./data/large_cluster.csv \
-t ./data/epigenome_2000.csv \
-p epigenome
echo "-------------------- CyberShake small --------------------"
$1/static_task_scheduling \
-c ./data/small_cluster.csv \
-t ./data/cybershake_100.csv \
-p cybershake
echo "-------------------- CyberShake large --------------------"
$1/static_task_scheduling \
-c ./data/large_cluster.csv \
-t ./data/cybershake_2000.csv \
-p cybershake
echo "-------------------- LIGO small --------------------"
$1/static_task_scheduling \
-c ./data/small_cluster.csv \
-t ./data/ligo_100.csv \
-p ligo
echo "-------------------- LIGO large --------------------"
$1/static_task_scheduling \
-c ./data/large_cluster.csv \
-t ./data/ligo_2000.csv \
-p ligo
echo "-------------------- Montage small --------------------"
$1/static_task_scheduling \
-c ./data/small_cluster.csv \
-t ./data/montage_100.csv \
-d ./data/montage_100.xml \
-p montage
echo "-------------------- Montage large --------------------"
$1/static_task_scheduling \
-c ./data/large_cluster.csv \
-t ./data/montage_1000.csv \
-d ./data/montage_1000.xml \
-p montage 
echo "-------------------- Missing topology (should error) --------------------"
$1/static_task_scheduling \
-c ./data/small_cluster.csv \
-t ./data/cybershake_100.csv
echo "-------------------- Montage missing topology (should error) --------------------"
$1/static_task_scheduling \
-c ./data/small_cluster.csv \
-t ./data/montage_100.csv \
-d ./data/montage_100.xml
echo "-------------------- Montage missing xml (should error) --------------------"
$1/static_task_scheduling \
-c ./data/small_cluster.csv \
-t ./data/montage_100.csv \
-p montage
echo "-------------------- Wrong topology (should error) --------------------"
$1/static_task_scheduling \
-c ./data/small_cluster.csv \
-t ./data/epigenome_100.csv \
-p ligo 
