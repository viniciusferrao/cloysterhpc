/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_REPOS_H_
#define CLOYSTERHPC_REPOS_H_

#include "os.h"
#include <string>

struct repofile {
    std::string id;
    bool enabled;
    std::string name;
    std::string baseurl;
    std::string metalink;
    bool gpgcheck;
    std::string gpgkey;
    std::string gpgkeyContent;
};

namespace ol {
const repofile ol8_base_latest
    = { "ol8_baseos_latest", true, "Oracle Linux 8 BaseOS Latest ($basearch)",
          "https://yum$ociregion.oracle.com/repo/OracleLinux/OL8/baseos/latest/"
          "$basearch/",
          "", true, "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-oracle",
          R"PGPKEY(-----BEGIN PGP PUBLIC KEY BLOCK-----
            Version: GnuPG v2.0.14 (GNU/Linux)

            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
            =dTZ0
            -----END PGP PUBLIC KEY BLOCK-----
            )PGPKEY" };
}

namespace rocky {
const repofile rocky8_baseos = { "Rocky-BaseOS", true,
    "Rocky Linux $releasever - BaseOS",
    "http://dl.rockylinux.org/$contentdir/$releasever/BaseOS/$basearch/os/", "",
    true, "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-rockyofficial",
    R"PGPKEY(-----BEGIN PGP PUBLIC KEY BLOCK-----

            mQINBGAofzYBEAC6yS1azw6f3wmaVd//3aSy6O2c9+jeetulRQvg2LvhRRS1eNqp
            /x9tbBhfohu/tlDkGpYHV7diePgMml9SZDy1sKlI3tDhx6GZ3xwF0fd1vWBZpmNk
            D9gRkUmYBeLotmcXQZ8ZpWLicosFtDpJEYpLUhuIgTKwt4gxJrHvkWsGQiBkJxKD
            u3/RlL4IYA3Ot9iuCBflc91EyAw1Yj0gKcDzbOqjvlGtS3ASXgxPqSfU0uLC9USF
            uKDnP2tcnlKKGfj0u6VkqISliSuRAzjlKho9Meond+mMIFOTT6qp4xyu+9Dj3IjZ
            IC6rBXRU3xi8z0qYptoFZ6hx70NV5u+0XUzDMXdjQ5S859RYJKijiwmfMC7gZQAf
            OkdOcicNzen/TwD/slhiCDssHBNEe86Wwu5kmDoCri7GJlYOlWU42Xi0o1JkVltN
            D8ZId+EBDIms7ugSwGOVSxyZs43q2IAfFYCRtyKHFlgHBRe9/KTWPUrnsfKxGJgC
            Do3Yb63/IYTvfTJptVfhQtL1AhEAeF1I+buVoJRmBEyYKD9BdU4xQN39VrZKziO3
            hDIGng/eK6PaPhUdq6XqvmnsZ2h+KVbyoj4cTo2gKCB2XA7O2HLQsuGduHzYKNjf
            QR9j0djjwTrsvGvzfEzchP19723vYf7GdcLvqtPqzpxSX2FNARpCGXBw9wARAQAB
            tDNSZWxlYXNlIEVuZ2luZWVyaW5nIDxpbmZyYXN0cnVjdHVyZUByb2NreWxpbnV4
            Lm9yZz6JAk4EEwEIADgWIQRwUcRwqSn0VM6+N7cVr12sbXRaYAUCYCh/NgIbDwUL
            CQgHAgYVCgkICwIEFgIDAQIeAQIXgAAKCRAVr12sbXRaYLFmEACSMvoO1FDdyAbu
            1m6xEzDhs7FgnZeQNzLZECv2j+ggFSJXezlNVOZ5I1I8umBan2ywfKQD8M+IjmrW
            k9/7h9i54t8RS/RN7KNo7ECGnKXqXDPzBBTs1Gwo1WzltAoaDKUfXqQ4oJ4aCP/q
            /XPVWEzgpJO1XEezvCq8VXisutyDiXEjjMIeBczxb1hbamQX+jLTIQ1MDJ4Zo1YP
            zlUqrHW434XC2b1/WbSaylq8Wk9cksca5J+g3FqTlgiWozyy0uxygIRjb6iTzKXk
            V7SYxeXp3hNTuoUgiFkjh5/0yKWCwx7aQqlHar9GjpxmBDAO0kzOlgtTw//EqTwR
            KnYZLig9FW0PhwvZJUigr0cvs/XXTTb77z/i/dfHkrjVTTYenNyXogPtTtSyxqca
            61fbPf0B/S3N43PW8URXBRS0sykpX4SxKu+PwKCqf+OJ7hMEVAapqzTt1q9T7zyB
            QwvCVx8s7WWvXbs2d6ZUrArklgjHoHQcdxJKdhuRmD34AuXWCLW+gH8rJWZpuNl3
            +WsPZX4PvjKDgMw6YMcV7zhWX6c0SevKtzt7WP3XoKDuPhK1PMGJQqQ7spegGB+5
            DZvsJS48Ip0S45Qfmj82ibXaCBJHTNZE8Zs+rdTjQ9DS5qvzRA1sRA1dBb/7OLYE
            JmeWf4VZyebm+gc50szsg6Ut2yT8hw==
            =AiP8
            -----END PGP PUBLIC KEY BLOCK-----
            )PGPKEY" };
}

class Repos {
private:
    std::string m_filename;
    std::string m_id;
    bool m_enabled;
    std::string m_name;
    std::string m_baseurl;
    std::string m_metalink;
    bool m_gpgcheck;
    std::string m_gpgkey;
    std::string m_gpgkeyPath; /* Location of the keyfile on filesystem */
    void configureRHEL() const;
    void configureRocky() const;
    void configureOL() const;
    void configureXCAT() const;
    OS::Distro m_distro;

public:
    explicit Repos(OS::Distro);
    void createConfigurationFile(const repofile& repo) const;
    void enable(const std::string&);
    void disable(const std::string&);
    void configureRepositories() const;
};

#endif // CLOYSTERHPC_REPOS_H_