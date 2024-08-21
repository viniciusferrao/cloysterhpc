ansible-playbook setup.yml --extra-vars "cloyster_binary_path=/path/to/local/cloyster/binary vagrant_machine_name=machine_name iso_image_path=/path/to/iso/image"

If you don't want to clean up after Cloyster run, set "cleanup_needed=false".