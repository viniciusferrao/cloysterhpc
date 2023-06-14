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
const repofile alma8_baseos = { "AlmaLinux-BaseOS", true,
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

namespace addons {
const repofile beegfs = { "beegfs-7.3.3-rhel8", true, "BeeGFS 7.3.3 (rhel8)",
    "https://mirror.versatushpc.com.br/beegfs/beegfs_7.3.3/dists/rhel8/", "",
    true, "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-beegfs_7.3.3",
    R"PGPKEY(-----BEGIN PGP PUBLIC KEY BLOCK-----

            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
            eAA803kyjtUmppWom/VgR4x/
            =3yN5
            -----END PGP PUBLIC KEY BLOCK-----
            )PGPKEY" };

const repofile grafana = { "grafana", true, "grafana",
    "https://mirror.versatushpc.com.br/grafana/", "", true,
    "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-grafana",
    R"PGPKEY(-----BEGIN PGP PUBLIC KEY BLOCK-----

            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
            =lSMJ
            -----END PGP PUBLIC KEY BLOCK-----
            )PGPKEY" };

const repofile influxdata
    = { "influxdata", true, "InfluxData Repository - Stable",
          "https://mirror.versatushpc.com.br/influxdata/", "", true,
          "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-influxdata",
          R"PGPKEY(-----BEGIN PGP PUBLIC KEY BLOCK-----

            mQINBGPIEycBEACpG4qSjhxA6fh4QJVJxFVBvCFt9tVx/hDbKH0Ryy9iilyMeReC
            AS1/CZnSv/fhDNKmVPckf6on72z/ODwZcVfMV6DHkxmZ6x/tQrS6CWfKkupsON2H
            KS3t4HUivahwHPlWtbfDqsWNwTAsZqklKpJQWY2ADPwurkbCmtYSjsgbLuWe23Pd
            nJpLTHtlChM0ntW/l7Le1zYjGPUGoxMJgjg1YG8fi2l/zS0Of8bdQ26ps+WRvrSQ
            RKhfAkfIgUiCXxBpDlN1spN73ZlAkaSb+myTfEKyJR55Yt9pHfkDdJh26RVgE1+N
            GuLmm6oidaD9lTlNJ9P8wlLzoof3xJXYprgLLz/HmgtawnJ+DxFIXoXNNpUmhORJ
            6Hb2Z5IKIyGIwXhQVe2Lw7B8awBNV99zUw517Wuax3RYx7Hwhntz9gFxS4GRxaCo
            uLCFQ0AgDCkMHyEHufQo1XdjIB7fz6U551y5GMQw6/rjMnUM9ZI68SQ/FWou2cQf
            533PyayvWOYQM4pP7ZmbzyCd393XlMaPWA5dyUOqv7Vcmv0IsAbncX6/KJmZAhKG
            qu19xb6rv3ab2RbcU422guK3C/h/URPZJbSjf2w4jUV5UDe2veZg6BEVn7Sk5bW0
            ceX8n0GVbPNG7CvRduJPjXNzsz3FzmUS8QFFde3H5gl1T0f6GcfhmKgKEQARAQAB
            tDdJbmZsdXhEYXRhIFBhY2thZ2UgU2lnbmluZyBLZXkgPHN1cHBvcnRAaW5mbHV4
            ZGF0YS5jb20+iQJVBBMBCAA/BgsJCAcDAgYVCAIJCgsEFgIDAQIeAQIXgAUJBaOk
            /BYhBJ1TnZDTMo3H1sjTudj/jh99+LB+BQJjyB9PAhsDAAoJENj/jh99+LB+klgQ
            AKOKdwTyKOr6+mnRrACz5U3EFxfAXXFGan9Ka7Nzgz4K+FOnTtT1gWwqrPPmTKQk
            epNUMcelfX1kCA08yCm0nyw2niqxES40W33ergKUj6jlDx7UQYXWsDQGD9IKksa8
            MWfZlJ3zlrsGKXA4oa+kfY+vltWDVP8WhLcQzm2LywbKvr3WgY80GZbnRjoekiBK
            oMKztQVMJG5yNZBo9B4JrqB3wMpnXZxEtqZcBPsJJdXTFKHsQ7kB9TMNorbUvDNH
            ohwsprgMw84vHikEk9jyCypXpYq/E/wvkM0CeIUJ36S2vGvACib7BiY6Xv0BQbM4
            rWq2Rrjag1y5vVAF9gJkeo/3rhM6lE1ahDCRq0QcBMVzbxiE+3COIzRPmz14J3Yn
            0pkvzlVkNj5UZR8q91ESl+UxkFCP1wzcXgs0dpJWirQIOZ9E2eYv3LcjE68xjW1k
            c5q1GOGvJI7aXADxUZ4lFbz+NUb4Ts4HXHc8gV1Gm0vvmIqv2YfAvL5DXbKLdZxh
            73CxKvBMmTXIEQ+vQJ3p1ZnUnb+l6DoxEFWg/hXHmE5jY3P6HIVFdliXF5FEs1lr
            9snU2Pn1BDL+TBN7SX0QbKqArWA4qyn6eGH8Z1ULoUVBPCjwC9QuInp/9fqifFYo
            OM3A51MDGyc/HCVG6jNJEI5h71QGHlPfyQybpjy7rQSe
            =YwXc
            -----END PGP PUBLIC KEY BLOCK-----
            )PGPKEY" };

const repofile oneAPI = { "oneAPI", true, "Intel® oneAPI repository",
    "https://mirror.versatushpc.com.br/oneAPI/", "", true,
    "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-oneAPI",
    R"PGPKEY(-----BEGIN PGP PUBLIC KEY BLOCK-----

            mQENBFfriU0BCADD98BaTc+sNO2nk7UwfhNIwiyN+8PtoozyIQtITGViuLGTC2f7kcA08blM+pkz
            qGc9+uYosY5zW++HPowg+b1IEvPAGv1/WzDgCcm8vMbvmgs9GQL64X81mkXglGO1xI7VFQ5KxvQY
            oaOA4NFSw0L3RTYyGl2AAGRbkEmdUg+6DCUZ5aNp+ZUhH5sV3rpUwWj8aJw/1SUm8KmA6GTb8a+s
            hQhygablAVyxY29/F4reIYD3TJsnKl9xcoc+NMaT76rOaL9TU71dhn9DRl6I2S63r4QmZXIgcikt
            vzbWwny8IBPFrh/5HaVh+eGIVg7aC/6ZPvwLzVTiKtywP3JP9OXzABEBAAG1AEIiQ04gPSBJbnRl
            bChSKSBTb2Z0d2FyZSBEZXZlbG9wbWVudCBQcm9kdWN0cyIsIE89SW50ZWwgQ29ycG9yYXRpb26J
            AT4EEwEIACgFAlfriU0CGwMFCQWiSQAGCwkIBwMCBhUIAgkKCwQWAgMBAh4BAheAAAoJEBqEl7EZ
            EeCX1+gH/1ChlnCBN9SqB1xKW6gPPX+9UcqTD+kr1HPve64S0eSoV86U5iKNPVdJ3GcRrEd/hNuh
            eiazQaxLjHBIkHMbur1x3/91uYFdVOndwKJwIRKZmP2vCjucNb8FO3I4lINevVn7IHTf4hMMyFEd
            0ZLBvlmxXqOlV2Tb4ACGFPhLLhKNvkf6TFCCc0VoYmcyNecWEoh2ZZ9w/+uuY2pKkNulrjPFWjKA
            G1TEIim+KkpzoBjBWAlb+Rf0OkahAWjc0LCbiA22oIdJxwsWYGqlJMSXCB5Q+Vk8duHRIm6tf5L3
            7ymDS8lnPgq3871dhD+i5K6LTXdlY9Yk2Nv2BbDLSm42l+A=
            =r6NW
            -----END PGP PUBLIC KEY BLOCK-----
            -----BEGIN PGP PUBLIC KEY BLOCK-----

            mQENBF2Ru1cBCADET6CERkWEDRAF6GIEceiajgMFT6BepRjF26rtnyEXdAXT4vU+wLqg0DE5Va1t
            4ZAXcIHJ3g1OuGw7Vst2b7WySJl1cFvRSOxZg7Em0QUd51c7n0IYX6FL55Ern2pU3sL2NfJnyrKi
            HBSTBGqMFELF5cTaY4E1GQyIUj0BrwUXMpLuxxL0ISyz2CNFnJuENbunbax0L5LrHLFwcsTEvT9q
            ZVcP5G+xZBmhPOJ3bR3LHZN/9tj1E6IEzFOY/S/jfqXKmwbR4EzZ7pTtBqNzDm6bviWwQHyh6euw
            eoDhTa6TAUUsP5vIL+7iWDQd9X10XyByWEsLaQbWNBCPxAwRTAgnABEBAAG1ACZJbnRlbChSKSBT
            b2Z0d2FyZSBEZXZlbG9wbWVudCBQcm9kdWN0c4kBPgQTAQgAKAUCXZG7VwIbAwUJB4YfgAYLCQgH
            AwIGFQgCCQoLBBYCAwECHgECF4AACgkQrPqfxX5sXb7L5Qf/cxJs9tDCea0ZWqeSX1lAjuff2Bku
            H/yXwvu625DcAneUemho6NDck2LJKhpUM/9teyQWGTIIuz8IlIr3zYeK8VmVitm6cgjD07TYeh5j
            71eAd7R1o49IUhIx/cXcuBcYHOw9kWCpl05D85xByZxkF7tlShLwVveyNGeVzU8aBJPhqoPGzIP0
            /CJ3c2Qe09UMmqyhUw5ETMqBxR1E+vFCqAHfa3VdtOc8gIfmMK50t/wjE33RdnMI0cHoqmRLFj6C
            ppBjBabIY+ro1focySyJ8cLkVEsk0T6u0s2B8nj+ctjasYAeZCmk6shNh9Y06zHkRcBS/cy2bHeB
            4nKs3YBvpA==
            =aXpT
            -----END PGP PUBLIC KEY BLOCK-----
            -----BEGIN PGP PUBLIC KEY BLOCK-----

            mQENBF07HV4BCADOChlmn/KbyUa5cffv4VlKfeI29FiOqXYW1AFhJLTmzvzTSTI6K1pdJKfZAT71
            9uUz6p0lUkbjyTTgUOwQbXULyehGqe0dHQ8rVU1NPzWkmlu/Z1P7WWyd/z1gNRKXz0BcGE0HUfdh
            IPSlNykqO1yaJfO55jT6KGjZHe4caR125my9EvwA9zAmF5MGQdNZPPnuh1VIEzqJAuPU/3hmBXjK
            epsMO6Wz1IXCvFByAvsLLDHxRag1ImBzZ6zc2LQuGGcZfjuH0cnJ1moxKEbfl13eGTCCgKN2nVPj
            b0G2uHY2xaWQMNBvYwuCWl4X7/iltJL21L0TNhECbiXgpvSS6Wd1ABEBAAG1ABVLRVktUElEVC1Q
            R1AtMjAxOTA3MjaJAT4EEwEIACgFAl07HV4CGwMFCQeGH4AGCwkIBwMCBhUIAgkKCwQWAgMBAh4B
            AheAAAoJEDa5Vps/GhvHMAUH/RwVG9JXvjLZ50nTo92Anm0cwdGq4Mb0Dr9eTg7xLxYiwV2TZ9Et
            iSrjrmpvKXeboJBES8fFx4AtkOMq7t25b1HTq72UTeYhVaddjQ7saf0W/mxjvGIKJgJPtQKZuxDa
            X5f4IQrhxbsR6rgXM4MOUR2ru+ejfZc6MQ7uQ1TKlA3/Z3aq1EXVzVCOzcg+N7Tz0VoqrRROuX0Z
            +qFqK5QW3u/G9ORpSQq1C/ATI2+4qmSvFixoyx+FvIxqtozj5hXSG7cEaS8fGCdOjJhw1QXudfAr
            izojEplMaapSKWoe3i0P+x5DdGBWuxLhA8iJt150rU4QMrh14FN7xLHBMY/7QW4=
            =rudQ
            -----END PGP PUBLIC KEY BLOCK-----
            -----BEGIN PGP PUBLIC KEY BLOCK-----

            mQENBF7Cku4BCADJYMbAmWE7IKY3QCIWm6EDSA4zvE2nj9mxOaTa02ZxXilEhQuU7pJlubqSbpPB
            /+und2L0ITRnZgYCDNNH3702bFVZPjgy/6R4ruZpz0GjlwijHIwLSS57irkXjF6RHB8VFhNy7949
            ua5a1NuBFvdRiaxSGE2mBv8pXrwiMqrS5mGXLVk6niS6zukVeQsxMZrdz1ScXvFeivotjbeTJu5K
            S4fuPzUTieNwrB519V4zvs5WNZIT0WipQxh/AODJK5jFug7zdCtOFv4YOZ1dlSciO4hxmHo8hvb8
            8mWHYmtBbZNWlpG1Aw6tmsKp3jVP/QvYaXZBdzoeEtxCCJBmxxxnABEBAAG1ADxDTj1JbnRlbChS
            KSBTb2Z0d2FyZSBEZXZlbG9wbWVudCBQcm9kdWN0cyAoUFJFUFJPRCBVU0UgT05MWSmJAT4EEwEI
            ACgFAl7Cku4CGwMFCQeGH4AGCwkIBwMCBhUIAgkKCwQWAgMBAh4BAheAAAoJEGQHNkJ4cqIgc00H
            /3aiK0HUaUh14TowUoKHxtD3tFNCRZ0BJIVmkxCKeKyqsFyUuIR7Ju36wgJjlRZgFaQo9eIgrTcq
            AjpFbXQOoeKjSjwP4O2i8/N75Sb+FRemP2WVz78DQxBEJAMjOydc/J1QMtWcu2jVXmrrCxZBK7Nl
            3TN/ZmQ5bgQ7Ifw1RFKtZCgGE50lRu2Zwcu4MKlIrIsovhds7HYCHhvG8VBEcoO5kqAVEeNr8iPX
            1W/f+e8c3inRSwGHt4P9O/cAeW/OVGXllXKwpbVVmEVErfwGgJmR+9syBZZv9Kb6KWgeP/y9091E
            wlB39FLnH/DsLu9AwNHGwDgmKTWQcwWhMQjnxRI=
            =7ONs
            -----END PGP PUBLIC KEY BLOCK-----
            )PGPKEY" };

const repofile zabbix = { "zabbix", true, "zabbix",
    "https://mirror.versatushpc.com.br/zabbix/zabbix/6.4/rhel/8/x86_64/", "",
    true, "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-ZABBIX",
    R"PGPKEY(-----BEGIN PGP PUBLIC KEY BLOCK-----
            Version: GnuPG v1.4.10 (GNU/Linux)

            mQGiBFCNJaYRBAC4nIW8o2NyOIswb82Xn3AYSMUcNZuKB2fMtpu0WxSXIRiX2BwC
            YXx8cIEQVYtLRBL5o0JdmoNCjW6jd5fOVem3EmOcPksvzzRWonIgFHf4EI2n1KJc
            JXX/nDC+eoh5xW35mRNFN/BEJHxxiRGGbp2MCnApwgrZLhOujaCGAwavGwCgiG4D
            wKMZ4xX6Y2Gv3MSuzMIT0bcEAKYn3WohS+udp0yC3FHDj+oxfuHpklu1xuI3y6ha
            402aEFahNi3wr316ukgdPAYLbpz76ivoouTJ/U2MqbNLjAspDvlnHXXyqPM5GC6K
            jtXPqNrRMUCrwisoAhorGUg/+S5pyXwsWcJ6EKmA80pR9HO+TbsELE5bGe/oc238
            t/2oBAC3zcQ46wPvXpMCNFb+ED71qDOlnDYaaAPbjgkvnp+WN6nZFFyevjx180Kw
            qWOLnlNP6JOuFW27MP75MDPDpbAAOVENp6qnuW9dxXTN80YpPLKUxrQS8vWPnzkY
            WtUfF75pEOACFVTgXIqEgW0E6oww2HJi9zF5fS8IlFHJztNYtbQgWmFiYml4IFNJ
            QSA8cGFja2FnZXJAemFiYml4LmNvbT6IYAQTEQIAIAUCUI0lpgIbAwYLCQgHAwIE
            FQIIAwQWAgMBAh4BAheAAAoJENE9WOR56l7UhUwAmgIGZ39U6D2w2oIWDD8m7KV3
            oI06AJ9EnOxMMlxEjTkt9lEvGhEX1bEh7bkBDQRQjSWmEAQAqx+ecOzBbhqMq5hU
            l39cJ6l4aocz6EZ9mSSoF/g+HFz6WYnPAfRaYyfLmZdtF5rGBDD4ysalYG5yD59R
            Mv5tNVf/CEx+JAPMhp6JCBkGRaH+xHws4eBPGkea4rGNVP3L3rA7g+c1YXZICGRI
            OOH7CIzIZ/w6aFGsPp7xM35ogncAAwUD/3s8Nc1OLDy81DC6rGpxfEURd5pvd/j0
            D5Di0WSBEcHXp5nThDz6ro/Vr0/FVIBtT97tmBHX27yBS3PqxxNRIjZ0GSWQqdws
            Q8o3YT+RHjBugXn8CzTOvIn+2QNMA8EtGIZPpCblJv8q6MFPi9m7avQxguMqufgg
            fAk7377Rt9RqiEkEGBECAAkFAlCNJaYCGwwACgkQ0T1Y5HnqXtQx4wCfcJZINKVq
            kQIoV3KTQAIzr6IvbZoAn12XXt4GP89xHuzPDZ86YJVAgnfK
            =+200
            -----END PGP PUBLIC KEY BLOCK-----
            )PGPKEY" };

const repofile ELRepo = { "elrepo", true, "elrepo",
    "https://mirror.versatushpc.com.br/elrepo/elrepo/el8/x86_64/", "", true,
    "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-elrepo.org",
    R"PGPKEY(-----BEGIN PGP PUBLIC KEY BLOCK-----
            Version: GnuPG v1.4.5 (GNU/Linux)

            mQGiBEm+/6QRBAC5mbtqOFSQ0FkTLIMdIoqxtraIeUqwbPp3IBYQ/u/EREjyEf1D
            qFyBEXII0dD8JDT85vRZ81jhB7nFWa0VbUfY0xfghkbnokiNBVNpiQcvszw3UYDF
            aLAaOC8Z98vmlsQaBBTQG6704ZXLr7FJyG3GP5WE6egXIQQbrMcdmCoRBwCg/dwC
            HLWjuemoDc5SX7hKHbB4zZ8D/jP+oMbqz+bDn8OZ2UuaGdxr+mHW8tzTdPjnEU8e
            hUt1ws8eBqn/gMfKvUBa8xFSILe8Ty99u+VjFbcRsdf0H6dRre9AdDVUz5oxzoPw
            gamA8mhPQvFh3wt9smtRUh5IoM2LiM1s5pGMLuYuvSnVUPArEnSfW6K5I6v7OarU
            3WfrBACDEGGcaWKjfdkRtmKIQrzu6AnldVC1ISLVAoqxHnKNFTk1BgO0PSZDpfJI
            x8fMCnGlusoL6F5+LYEk4K4B0zvlj1ur3JocjxpuBLccl94JTo/+I9ZbS8ptUqLw
            LBUkgIQJzzIH4G5NZsQ3FpzSWGRFVa7etqTv9BfUMUmJxhEoobQ/ZWxyZXBvLm9y
            ZyAoUlBNIFNpZ25pbmcgS2V5IGZvciBlbHJlcG8ub3JnKSA8c2VjdXJlQGVscmVw
            by5vcmc+iGAEExECACAFAkm+/6QCGwMGCwkIBwMCBBUCCAMEFgIDAQIeAQIXgAAK
            CRAwm8MFuq2uUlgWAKCGWOpyodbzxS7Xy/0X9m9qVnHq+ACfUHrQzYAtFRpT07Sg
            giosE+mvAKu5Ag0ESb7/pxAIALgT0q0HhwugE717a7N+gAtCbFu8pCXOZcrAnJpG
            cMA9VWmsODZStPTxXYM2ggCMSzUnNis8pPZOPIP5C+t2IYtVjLshM4C9UiY7u5ET
            jErWCxWmF+2QLO50K56E4qxj0wufZt9P+Ih0USUM5L2oyrQG51uj/2v3Qq3igc8Z
            NTtmEOfis3losusQbAfZtTBmNQ0570kkhMxiyavgAUxLenXHYrkDJFuL7XdBCmna
            kykTn2dzU81rIDZ+QPxII4V/eZ5xGiRY/EDUIAysEV2m0NNZgWi/twUnZICm7zYp
            VRviJrBnFTvNEPMhiGRnJgQp/Krv4WIHQ67N8lQg3q5RJX8AAwUH/0UBjBgbsuWR
            dB+ZYWjKPBy+bJY/6HefPUuLrt3QDNINMW8kY7VzWRMWyGc4IlPJDW0nwOn/vT5H
            Dgc3YmA3tm7cKj5wpFijzff61Db8eq+CUKGscKxDBGzHq6oirM5U7DQLmipCs5Eb
            efwHIjE3aOUSnoQmniEnBWI2hm/rftmY9oJSi8xgns4wAokDPiMcucADtbV3fznx
            ppuowtBi8bcGB1vJZjTySQcSKWv+IVp5ej06uZ9501BEm6HxUZkuAGPecP65kcQu
            5bg7B7LQeI5AWyujqvfuWgAF34xhl9QU/sDWxM3wUog+bw7y8pyWCqnJhp70yx8o
            SqzhFygT62eISQQYEQIACQUCSb7/pwIbDAAKCRAwm8MFuq2uUq8PAKC1+E2pSwiS
            oHXkKYPYDwApsP1mVACfRe1YnggLYQtG9LMeweVQQC77rK8=
            =qyRr
            -----END PGP PUBLIC KEY BLOCK-----
            )PGPKEY" };

// @TODO add rpmforge
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
    void configureAddons() const;
    OS::Distro m_distro;

public:
    explicit Repos(OS::Distro);
    void createConfigurationFile(const repofile& repo) const;
    void enable(const std::string&);
    void disable(const std::string&);
    void configureRepositories() const;
};

#endif // CLOYSTERHPC_REPOS_H_