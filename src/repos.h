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

namespace alma {
const repofile alma8_baseos = { "Almalinux-BaseOS", true,
    "AlmaLinux $releasever - BaseOS",
    "https://repo.almalinux.org/almalinux/$releasever/BaseOS/$basearch/os/", "",
    true, "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-AlmaLinux",
    R"PGPKEY(-----BEGIN PGP PUBLIC KEY BLOCK-----
            Version: GnuPG v1

            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
            72EHtteVBoVn0eg6
            =rEWJ
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
    void configureAlma() const;
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